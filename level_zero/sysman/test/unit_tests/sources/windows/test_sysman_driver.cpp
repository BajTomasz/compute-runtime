/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/sysman/test/unit_tests/sources/windows/mock_sysman_driver.h"

#include "gtest/gtest.h"

#include <bitset>
#include <cstring>

namespace L0 {
namespace Sysman {
namespace ult {

TEST(zesInit, whenCallingZesInitThenInitializeOnDriverIsCalled) {
    MockSysmanDriver driver;

    auto result = zesInit(ZE_INIT_FLAG_GPU_ONLY);
    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(1u, driver.initCalledCount);
}

TEST(zesInit, whenCallingZesInitWithNoFlagsThenInitializeOnDriverIsCalled) {
    MockSysmanDriver driver;

    auto result = zesInit(0);
    EXPECT_EQ(ZE_RESULT_SUCCESS, result);
    EXPECT_EQ(1u, driver.initCalledCount);
}

TEST(zesInit, whenCallingZesInitWithoutGpuOnlyFlagThenInitializeOnDriverIsNotCalled) {
    MockSysmanDriver driver;

    auto result = zesInit(ZE_INIT_FLAG_VPU_ONLY);
    EXPECT_EQ(ZE_RESULT_ERROR_UNINITIALIZED, result);
    EXPECT_EQ(0u, driver.initCalledCount);
}

} // namespace ult
} // namespace Sysman
} // namespace L0
