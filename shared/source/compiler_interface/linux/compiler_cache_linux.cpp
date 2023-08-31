/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/compiler_interface/compiler_cache.h"
#include "shared/source/compiler_interface/os_compiler_cache_helper.h"
#include "shared/source/debug_settings/debug_settings_manager.h"
#include "shared/source/helpers/file_io.h"
#include "shared/source/helpers/string.h"
#include "shared/source/os_interface/linux/sys_calls.h"
#include "shared/source/utilities/io_functions.h"

#include "os_inc.h"

#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string_view>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace NEO {
int filterFunction(const struct dirent *file) {
    std::string_view fileName = file->d_name;
    if (fileName.find(".cl_cache") != fileName.npos || fileName.find(".l0_cache") != fileName.npos) {
        return 1;
    }

    return 0;
}

struct ElementsStruct {
    std::string path;
    struct stat statEl;
};

bool compareByLastAccessTime(const ElementsStruct &a, ElementsStruct &b) {
    return a.statEl.st_atime < b.statEl.st_atime;
}

bool CompilerCache::evictCache() {
    struct dirent **files = 0;

    int filesCount = NEO::SysCalls::scandir(config.cacheDir.c_str(), &files, filterFunction, NULL);

    if (filesCount == -1) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Scandir failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
        return false;
    }

    std::vector<ElementsStruct> vec;
    vec.reserve(static_cast<size_t>(filesCount));
    for (int i = 0; i < filesCount; ++i) {
        ElementsStruct fileElement = {};
        fileElement.path = makePath(config.cacheDir, files[i]->d_name);
        if (NEO::SysCalls::stat(fileElement.path.c_str(), &fileElement.statEl) == 0) {
            vec.push_back(std::move(fileElement));
        }
    }

    for (int i = 0; i < filesCount; ++i) {
        free(files[i]);
    }
    free(files);

    std::sort(vec.begin(), vec.end(), compareByLastAccessTime);

    size_t evictionLimit = config.cacheSize / 3;

    size_t evictionSizeCount = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
        NEO::SysCalls::unlink(vec[i].path);
        evictionSizeCount += vec[i].statEl.st_size;

        if (evictionSizeCount > evictionLimit) {
            return true;
        }
    }

    return true;
}

bool CompilerCache::createUniqueTempFileAndWriteData(char *tmpFilePathTemplate, const char *pBinary, size_t binarySize) {
    int fd = NEO::SysCalls::mkstemp(tmpFilePathTemplate);
    if (fd == -1) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Creating temporary file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
        return false;
    }
    if (NEO::SysCalls::pwrite(fd, pBinary, binarySize, 0) == -1) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Writing to temporary file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
        NEO::SysCalls::close(fd);
        NEO::SysCalls::unlink(tmpFilePathTemplate);
        return false;
    }

    return NEO::SysCalls::close(fd) == 0;
}

bool CompilerCache::renameTempFileBinaryToProperName(const std::string &oldName, const std::string &kernelFileHash) {
    int err = NEO::SysCalls::rename(oldName.c_str(), kernelFileHash.c_str());

    if (err < 0) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Rename temp file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
        NEO::SysCalls::unlink(oldName);
        return false;
    }

    return true;
}

void unlockFileAndClose(int fd) {
    int lockErr = NEO::SysCalls::flock(fd, LOCK_UN);

    if (lockErr < 0) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: unlock file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
    }

    NEO::SysCalls::close(fd);
}

void CompilerCache::lockConfigFileAndReadSize(const std::string &configFilePath, int &fd, size_t &directorySize) {
    bool countDirectorySize = false;
    errno = 0;
    fd = NEO::SysCalls::open(configFilePath.c_str(), O_RDWR);

    if (fd < 0) {
        if (errno == ENOENT) {
            fd = NEO::SysCalls::openWithMode(configFilePath.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            if (fd < 0) {
                fd = NEO::SysCalls::open(configFilePath.c_str(), O_RDWR);
            } else {
                countDirectorySize = true;
            }
        } else {
            NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Open config file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
            return;
        }
    }

    int lockErr = NEO::SysCalls::flock(fd, LOCK_EX);

    if (lockErr < 0) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Lock config file failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
        NEO::SysCalls::close(fd);
        fd = -1;

        return;
    }

    if (countDirectorySize) {
        struct dirent **files = {};

        int filesCount = NEO::SysCalls::scandir(config.cacheDir.c_str(), &files, filterFunction, NULL);

        if (filesCount == -1) {
            unlockFileAndClose(fd);
            NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Scandir failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
            fd = -1;
            return;
        }

        std::vector<ElementsStruct> vec;
        vec.reserve(static_cast<size_t>(filesCount));
        for (int i = 0; i < filesCount; ++i) {
            std::string_view fileName = files[i]->d_name;
            if (fileName.find(config.cacheFileExtension) != fileName.npos) {
                ElementsStruct fileElement = {};
                fileElement.path = makePath(config.cacheDir, files[i]->d_name);
                if (NEO::SysCalls::stat(fileElement.path.c_str(), &fileElement.statEl) == 0) {
                    vec.push_back(std::move(fileElement));
                }
            }
        }

        for (int i = 0; i < filesCount; ++i) {
            free(files[i]);
        }
        free(files);

        for (auto &element : vec) {
            directorySize += element.statEl.st_size;
        }

    } else {
        ssize_t readErr = NEO::SysCalls::pread(fd, &directorySize, sizeof(directorySize), 0);

        if (readErr < 0) {
            directorySize = 0;
            NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr, "PID %d [Cache failure]: Read config failed! errno: %d\n", NEO::SysCalls::getProcessId(), errno);
            unlockFileAndClose(fd);
            fd = -1;
        }
    }
}

bool CompilerCache::cacheBinary(const std::string &kernelFileHash, const char *pBinary, size_t binarySize) {
    if (pBinary == nullptr || binarySize == 0) {
        return false;
    }

    std::unique_lock<std::mutex> lock(cacheAccessMtx);
    constexpr std::string_view configFileName = "config.file";

    std::string configFilePath = makePath(config.cacheDir, configFileName.data());
    std::string filePath = makePath(config.cacheDir, kernelFileHash + config.cacheFileExtension);

    int fd = -1;
    size_t directorySize = 0u;

    lockConfigFileAndReadSize(configFilePath, fd, directorySize);

    if (fd < 0) {
        return false;
    }

    struct stat statbuf = {};
    if (NEO::SysCalls::stat(filePath, &statbuf) == 0) {
        unlockFileAndClose(fd);
        return true;
    }

    size_t maxSize = config.cacheSize;

    if (maxSize < directorySize + binarySize) {
        if (!evictCache()) {
            unlockFileAndClose(fd);
            return false;
        }
    }

    std::string tmpFileName = "cl_cache.XXXXXX";

    std::string tmpFilePath = makePath(config.cacheDir, tmpFileName);

    if (!createUniqueTempFileAndWriteData(tmpFilePath.data(), pBinary, binarySize)) {
        unlockFileAndClose(fd);
        return false;
    }

    if (!renameTempFileBinaryToProperName(tmpFilePath, filePath)) {
        unlockFileAndClose(fd);
        return false;
    }

    directorySize += binarySize;

    NEO::SysCalls::pwrite(fd, &directorySize, sizeof(directorySize), 0);

    unlockFileAndClose(fd);

    return true;
}

std::unique_ptr<char[]> CompilerCache::loadCachedBinary(const std::string &kernelFileHash, size_t &cachedBinarySize) {
    std::string filePath = makePath(config.cacheDir, kernelFileHash + config.cacheFileExtension);

    return loadDataFromFile(filePath.c_str(), cachedBinarySize);
}
} // namespace NEO
