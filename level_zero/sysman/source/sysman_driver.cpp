/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/debug_settings/debug_settings_manager.h"
#include "shared/source/execution_environment/execution_environment.h"
#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/os_interface/debug_env_reader.h"
#include "shared/source/os_interface/os_interface.h"

#include "level_zero/core/source/driver/driver.h"
#include "level_zero/sysman/source/sysman_driver_handle_imp.h"
#include "level_zero/sysman/source/sysman_driver_imp.h"

#include <cstring>
#include <vector>

namespace L0 {
namespace Sysman {
_ze_driver_handle_t *GlobalSysmanDriverHandle;
uint32_t driverCount = 1;

void SysmanDriverImp::initialize(ze_result_t *result) {
    *result = ZE_RESULT_ERROR_UNINITIALIZED;

    if (sysmanInitFromCore) {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr,
                              "%s", "Sysman Initialization already happened via zeInit\n");
        return;
    }

    auto executionEnvironment = new NEO::ExecutionEnvironment();
    UNRECOVERABLE_IF(nullptr == executionEnvironment);
    executionEnvironment->incRefInternal();

    using HwDeviceIds = std::vector<std::unique_ptr<NEO::HwDeviceId>>;

    HwDeviceIds hwDeviceIds = NEO::OSInterface::discoverDevices(*executionEnvironment);
    if (!hwDeviceIds.empty()) {
        executionEnvironment->prepareRootDeviceEnvironments(static_cast<uint32_t>(hwDeviceIds.size()));
        uint32_t rootDeviceIndex = 0u;
        for (auto &hwDeviceId : hwDeviceIds) {
            if (!executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->initOsInterface(std::move(hwDeviceId), rootDeviceIndex)) {
                NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr,
                                      "OsInterface initialization failed for device : %d\n", rootDeviceIndex);
                *result = ZE_RESULT_ERROR_UNINITIALIZED;
                executionEnvironment->decRefInternal();
                return;
            }
            rootDeviceIndex++;
        }

        GlobalSysmanDriverHandle = SysmanDriverHandle::create(*executionEnvironment, result);
    } else {
        NEO::printDebugString(NEO::DebugManager.flags.PrintDebugMessages.get(), stderr,
                              "%s\n", "No devices found");
        *result = ZE_RESULT_ERROR_UNINITIALIZED;
    }
    executionEnvironment->decRefInternal();
}

ze_result_t SysmanDriverImp::initStatus(ZE_RESULT_ERROR_UNINITIALIZED);

ze_result_t SysmanDriverImp::driverInit(zes_init_flags_t flags) {
    std::call_once(initDriverOnce, [this]() {
        ze_result_t result;
        this->initialize(&result);
        initStatus = result;
    });
    return initStatus;
}

ze_result_t driverHandleGet(uint32_t *pCount, zes_driver_handle_t *phDriverHandles) {
    if (*pCount == 0) {
        *pCount = driverCount;
        return ZE_RESULT_SUCCESS;
    }

    if (*pCount > driverCount) {
        *pCount = driverCount;
    }

    if (phDriverHandles == nullptr) {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    for (uint32_t i = 0; i < *pCount; i++) {
        phDriverHandles[i] = GlobalSysmanDriverHandle;
    }

    return ZE_RESULT_SUCCESS;
}

static SysmanDriverImp driverImp;
SysmanDriver *SysmanDriver::driver = &driverImp;

ze_result_t init(zes_init_flags_t flags) {
    if (flags && !(flags & ZE_INIT_FLAG_GPU_ONLY)) {
        return ZE_RESULT_ERROR_UNINITIALIZED;
    } else {
        return SysmanDriver::get()->driverInit(flags);
    }
}

} // namespace Sysman
} // namespace L0
