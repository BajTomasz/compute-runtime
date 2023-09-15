/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/release_helper/release_helper.h"

namespace NEO {

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isMatrixMultiplyAccumulateSupported() const {
    return true;
}
template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isAdjustWalkOrderAvailable() const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isPipeControlPriorToNonPipelinedStateCommandsWARequired() const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isProgramAllStateComputeCommandFieldsWARequired() const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isPrefetchDisablingRequired() const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isSplitMatrixMultiplyAccumulateSupported() const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isBFloat16ConversionSupported() const {
    return false;
}

template <ReleaseType releaseType>
int ReleaseHelperHw<releaseType>::getProductMaxPreferredSlmSize(int preferredEnumValue) const {
    return preferredEnumValue;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::getMediaFrequencyTileIndex(uint32_t &tileIndex) const {
    return false;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isResolvingSubDeviceIDNeeded() const {
    return true;
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::isCachingOnCpuAvailable() const {
    return true;
}

template <ReleaseType releaseType>
std::optional<GfxMemoryAllocationMethod> ReleaseHelperHw<releaseType>::getPreferredAllocationMethod(AllocationType allocationType) const {
    return {};
}

template <ReleaseType releaseType>
bool ReleaseHelperHw<releaseType>::shouldAdjustDepth() const {
    return false;
}
} // namespace NEO
