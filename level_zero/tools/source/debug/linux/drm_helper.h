/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/topology_map.h"

#include <string>

namespace NEO {
struct EngineClassInstance;
enum class DrmIoctl;
} // namespace NEO
namespace aub_stream {
enum EngineType : uint32_t;
}

namespace L0 {
struct Device;
struct DrmHelper {
    static int ioctl(Device *device, NEO::DrmIoctl request, void *arg);
    static std::string getSysFsPciPath(Device *device);
    static int getErrno(Device *device);
    static uint32_t getEngineTileIndex(Device *device, const NEO::EngineClassInstance &engine);
    static const NEO::EngineClassInstance *getEngineInstance(Device *device, uint32_t tile, aub_stream::EngineType engineType);
    static int getTileIdFromGtId(Device *device, int gtId);
};

} // namespace L0
