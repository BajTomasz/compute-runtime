/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/api_specific_config.h"
#include "shared/source/helpers/timestamp_packet.h"

#include "level_zero/core/source/event/event.h"

namespace L0 {

template <typename TagSizeT>
class KernelEventCompletionData : public NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount> {
  public:
    uint32_t getPacketsUsed() const { return packetsUsed; }
    void setPacketsUsed(uint32_t value) { packetsUsed = value; }

  protected:
    uint32_t packetsUsed = 1;
};

template <typename TagSizeT>
struct EventImp : public Event {

    EventImp(EventPool *eventPool, int index, Device *device, bool tbxMode)
        : Event(eventPool, index, device), tbxMode(tbxMode) {
        contextStartOffset = NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount>::getContextStartOffset();
        contextEndOffset = NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount>::getContextEndOffset();
        globalStartOffset = NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount>::getGlobalStartOffset();
        globalEndOffset = NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount>::getGlobalEndOffset();
        timestampSizeInDw = (sizeof(TagSizeT) / sizeof(uint32_t));
        singlePacketSize = NEO::TimestampPackets<TagSizeT, NEO::TimestampPacketConstants::preferredPacketCount>::getSinglePacketSize();

        if (NEO::ApiSpecificConfig::isDynamicPostSyncAllocLayoutEnabled()) {
            singlePacketSize = sizeof(uint64_t);
        }
    }

    ~EventImp() override {}

    ze_result_t hostSignal() override;

    ze_result_t hostSynchronize(uint64_t timeout) override;

    ze_result_t queryStatus() override;

    ze_result_t reset() override;

    ze_result_t queryKernelTimestamp(ze_kernel_timestamp_result_t *dstptr) override;
    ze_result_t queryTimestampsExp(Device *device, uint32_t *count, ze_kernel_timestamp_result_t *timestamps) override;
    ze_result_t queryKernelTimestampsExt(Device *device, uint32_t *pCount, ze_event_query_kernel_timestamps_results_ext_properties_t *pResults) override;

    void resetDeviceCompletionData(bool resetAllPackets);
    void resetKernelCountAndPacketUsedCount() override;

    uint64_t getPacketAddress(Device *device) override;
    uint32_t getPacketsInUse() const override;
    uint32_t getPacketsUsedInLastKernel() override;
    void setPacketsInUse(uint32_t value) override;

    std::unique_ptr<KernelEventCompletionData<TagSizeT>[]> kernelEventCompletionData;

    const bool tbxMode = false;

  protected:
    ze_result_t waitForUserFence(uint64_t timeout);

    bool handlePreQueryStatusOperationsAndCheckCompletion();

    ze_result_t calculateProfilingData();
    ze_result_t queryStatusEventPackets();
    ze_result_t queryInOrderEventStatus();
    void handleSuccessfulHostSynchronization();
    MOCKABLE_VIRTUAL ze_result_t hostEventSetValue(TagSizeT eventValue);
    ze_result_t hostEventSetValueTimestamps(TagSizeT eventVal);
    MOCKABLE_VIRTUAL void assignKernelEventCompletionData(void *address);
    void setRemainingPackets(TagSizeT eventVal, uint64_t nextPacketGpuVa, void *nextPacketAddress, uint32_t packetsAlreadySet);
    void getSynchronizedKernelTimestamps(ze_synchronized_timestamp_result_ext_t *pSynchronizedTimestampsBuffer,
                                         const uint32_t count, const ze_kernel_timestamp_result_t *pKernelTimestampsBuffer);
    void copyDataToEventAlloc(void *dstHostAddr, uint64_t dstGpuVa, size_t copySize, uint64_t copyData);
};

} // namespace L0