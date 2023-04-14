/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/non_copyable_or_moveable.h"

#include "level_zero/sysman/source/ecc/ecc.h"
#include "level_zero/sysman/source/os_sysman.h"

namespace L0 {
namespace Sysman {
class FirmwareUtil;
struct OsSysman;

class EccImp : public Ecc, NEO::NonCopyableOrMovableClass {
  public:
    void init() override {}
    ze_result_t deviceEccAvailable(ze_bool_t *pAvailable) override;
    ze_result_t deviceEccConfigurable(ze_bool_t *pConfigurable) override;
    ze_result_t getEccState(zes_device_ecc_properties_t *pState) override;
    ze_result_t setEccState(const zes_device_ecc_desc_t *newState, zes_device_ecc_properties_t *pState) override;

    EccImp(OsSysman *pOsSysman) : pOsSysman(pOsSysman){};
    ~EccImp() override {}

  private:
    OsSysman *pOsSysman = nullptr;
    FirmwareUtil *pFwInterface = nullptr;

    enum eccState : uint8_t {
        eccStateDisable = 0,
        eccStateEnable = 1,
        eccStateNone = 0xFF
    };

    zes_device_ecc_state_t getEccState(uint8_t state);
    static FirmwareUtil *getFirmwareUtilInterface(OsSysman *pOsSysman);
    ze_result_t getEccFwUtilInterface(FirmwareUtil *&pFwUtil);
};

} // namespace Sysman
} // namespace L0
