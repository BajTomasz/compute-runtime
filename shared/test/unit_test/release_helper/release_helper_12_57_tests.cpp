/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/release_helper/release_helper.h"
#include "shared/test/unit_test/release_helper/release_helper_tests_base.h"

#include "gtest/gtest.h"

struct ReleaseHelper1257Tests : public ReleaseHelperTests<12, 57> {

    std::vector<uint32_t> getRevisions() override {
        return {0};
    }
};

TEST_F(ReleaseHelper1257Tests, whenGettingCapabilitiesThenCorrectPropertiesAreReturned) {
    for (auto &revision : getRevisions()) {
        ipVersion.revision = revision;
        releaseHelper = ReleaseHelper::create(ipVersion);
        ASSERT_NE(nullptr, releaseHelper);

        EXPECT_FALSE(releaseHelper->isAdjustWalkOrderAvailable());
        EXPECT_TRUE(releaseHelper->isMatrixMultiplyAccumulateSupported());
        EXPECT_FALSE(releaseHelper->isPipeControlPriorToNonPipelinedStateCommandsWARequired());
        EXPECT_TRUE(releaseHelper->isProgramAllStateComputeCommandFieldsWARequired());
        EXPECT_FALSE(releaseHelper->isPrefetchDisablingRequired());
        EXPECT_TRUE(releaseHelper->isSplitMatrixMultiplyAccumulateSupported());
        EXPECT_TRUE(releaseHelper->isBFloat16ConversionSupported());
        EXPECT_TRUE(releaseHelper->isResolvingSubDeviceIDNeeded());
        EXPECT_TRUE(releaseHelper->isCachingOnCpuAvailable());
    }
}

TEST_F(ReleaseHelper1257Tests, whenGettingMaxPreferredSlmSizeThenSizeIsNotModified) {
    whenGettingMaxPreferredSlmSizeThenSizeIsNotModified();
}

TEST_F(ReleaseHelper1257Tests, whenGettingMediaFrequencyTileIndexThenFalseIsReturned) {
    whenGettingMediaFrequencyTileIndexThenFalseIsReturned();
}

TEST_F(ReleaseHelper1257Tests, whenGettingPreferredAllocationMethodThenNoPreferenceIsReturned) {
    whenGettingPreferredAllocationMethodThenNoPreferenceIsReturned();
}