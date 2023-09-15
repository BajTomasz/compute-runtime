/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/ail/ail_configuration.h"
#include "shared/test/common/helpers/default_hw_info.h"
#include "shared/test/common/helpers/variable_backup.h"
#include "shared/test/common/test_macros/hw_test.h"

namespace NEO {

using AILTestsMTL = ::testing::Test;

HWTEST2_F(AILTestsMTL, givenMtlWhenSvchostAppIsDetectedThenDisableDirectSubmission, IsMTL) {
    class AILMock : public AILConfigurationHw<productFamily> {
      public:
        using AILConfiguration::apply;
        using AILConfiguration::processName;
    };

    VariableBackup<AILConfiguration *> ailConfigurationBackup(&ailConfigurationTable[productFamily]);
    AILMock ail;
    ailConfigurationTable[productFamily] = &ail;

    auto capabilityTable = defaultHwInfo->capabilityTable;
    auto defaultEngineSupportedValue = capabilityTable.directSubmissionEngines.data[aub_stream::ENGINE_CCS].engineSupported;

    ail.processName = "UnknownProcess";
    ail.apply(capabilityTable);
    EXPECT_EQ(defaultEngineSupportedValue, capabilityTable.directSubmissionEngines.data[aub_stream::ENGINE_CCS].engineSupported);

    ail.processName = "svchost";
    ail.apply(capabilityTable);
    EXPECT_FALSE(capabilityTable.directSubmissionEngines.data[aub_stream::ENGINE_CCS].engineSupported);
}
} // namespace NEO