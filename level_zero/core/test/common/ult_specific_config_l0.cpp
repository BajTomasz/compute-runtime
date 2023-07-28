/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/base_ult_config_listener.h"
#include "shared/test/common/helpers/test_files.h"
#include "shared/test/common/tests_configuration.h"

#include "level_zero/core/source/cmdlist/cmdlist.h"

#include "gmock/gmock.h"

using namespace NEO;
void cleanTestHelpers() {}

bool sysmanUltsEnable = false;

void applyWorkarounds() {

    auto sysmanUltsEnableEnv = getenv("NEO_L0_SYSMAN_ULTS_ENABLE");
    if (sysmanUltsEnableEnv != nullptr) {
        sysmanUltsEnable = (strcmp(sysmanUltsEnableEnv, "1") == 0);
    }
    {
        class BaseClass {
          public:
            int method(int param) { return 1; }
        };
        class MockClass : public BaseClass {
          public:
            MOCK_METHOD1(method, int(int param));
        };
        ::testing::NiceMock<MockClass> mockObj;
        EXPECT_CALL(mockObj, method(::testing::_))
            .Times(1);
        mockObj.method(2);
    }
}

void initGTest(int &argc, char **argv) {
    testing::InitGoogleMock(&argc, argv);
}

bool isPlatformSupported(const HardwareInfo &hwInfoForTests) {
    return L0::commandListFactory[hwInfoForTests.platform.eProductFamily] && hwInfoForTests.capabilityTable.levelZeroSupported;
}

void setupTestFiles(std::string testBinaryFiles, int32_t revId) {
    std::string testBinaryFilesApiSpecific = testBinaryFiles;
    testBinaryFilesApiSpecific.append("/level_zero/");
    testBinaryFiles.append("/" + binaryNameSuffix + "/");
    testBinaryFilesApiSpecific.append(binaryNameSuffix + "/");

    testBinaryFiles.append(std::to_string(revId));
    testBinaryFiles.append("/");
    testBinaryFiles.append(testFiles);
    testBinaryFilesApiSpecific.append(std::to_string(revId));
    testBinaryFilesApiSpecific.append("/");
    testBinaryFilesApiSpecific.append(testFilesApiSpecific);
    testFiles = testBinaryFiles;
    testFilesApiSpecific = testBinaryFilesApiSpecific;
}

std::string getBaseExecutionDir() {
    if (testMode != TestMode::AubTests) {
        return "level_zero/";
    }
    return "";
}

void addUltListener(::testing::TestEventListeners &listeners) {
    listeners.Append(new BaseUltConfigListener);
}
