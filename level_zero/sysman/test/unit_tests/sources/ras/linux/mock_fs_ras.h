/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "level_zero/sysman/source/linux/sysman_fs_access.h"
#include "level_zero/sysman/source/ras/linux/sysman_os_ras_imp.h"
#include "level_zero/sysman/source/ras/sysman_ras.h"
#include "level_zero/sysman/source/ras/sysman_ras_imp.h"

namespace L0 {
namespace ult {

class MockRasFsAccess : public L0::Sysman::FsAccess {
  public:
    bool mockRootUser = true;
    bool isRootUser() override {
        return mockRootUser;
    }
    MockRasFsAccess() = default;
};

} // namespace ult
} // namespace L0
