/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "third_party/aub_stream/headers/aub_manager.h"
#include "third_party/aub_stream/headers/options.h"

namespace aub_stream {

MMIOList injectMMIOList;

AubManager *AubManager::create(uint32_t gfxFamily, uint32_t devicesCount, uint64_t memoryBankSizeInGB, bool localMemorySupported, const std::string &aubFileName) {
    return nullptr;
}

} // namespace AubDump
