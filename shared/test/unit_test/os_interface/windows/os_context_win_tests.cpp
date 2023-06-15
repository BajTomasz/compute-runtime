/*
 * Copyright (C) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/helpers/engine_descriptor_helper.h"
#include "shared/test/common/mocks/mock_wddm.h"
#include "shared/test/common/os_interface/windows/wddm_fixture.h"

using namespace NEO;

struct OsContextWinTest : public WddmTestWithMockGdiDll {
    void SetUp() override {
        WddmTestWithMockGdiDll::SetUp();
        preemptionMode = PreemptionHelper::getDefaultPreemptionMode(*defaultHwInfo);
        auto &gfxCoreHelper = this->rootDeviceEnvironment->getHelper<GfxCoreHelper>();
        engineTypeUsage = gfxCoreHelper.getGpgpuEngineInstances(*rootDeviceEnvironment)[0];

        init();
    }

    PreemptionMode preemptionMode;
    EngineTypeUsage engineTypeUsage;
};

TEST_F(OsContextWinTest, givenWddm20WhenCreatingOsContextThenOsContextIsInitialized) {
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_NO_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenWddm20WhenCreatingWddmContextFailThenOsContextCreationFails) {
    wddm->device = INVALID_HANDLE;
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_ANY_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenWddm20WhenCreatingWddmMonitorFenceFailThenOsContextCreationFails) {
    *getCreateSynchronizationObject2FailCallFcn() = true;
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_ANY_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenWddm20WhenRegisterTrimCallbackFailThenOsContextCreationFails) {
    *getRegisterTrimNotificationFailCallFcn() = true;
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_ANY_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenWddm20WhenRegisterTrimCallbackIsDisabledThenOsContextIsInitialized) {
    DebugManagerStateRestore stateRestore;
    DebugManager.flags.DoNotRegisterTrimCallback.set(true);
    *getRegisterTrimNotificationFailCallFcn() = true;

    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_NO_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenReinitializeContextWhenContextIsInitThenContextIsDestroyedAndRecreated) {
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_NO_THROW(osContext->reInitializeContext());
    EXPECT_NO_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenReinitializeContextWhenContextIsNotInitThenContextIsCreated) {
    EXPECT_NO_THROW(osContext->reInitializeContext());
    EXPECT_NO_THROW(osContext->ensureContextInitialized());
}

TEST_F(OsContextWinTest, givenOsContextWinWhenQueryingForOfflineDumpContextIdThenCorrectValueIsReturned) {
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_EQ(0u, osContext->getOfflineDumpContextId(0));
}

TEST_F(OsContextWinTest, givenWddm20AndProductSupportDirectSubmissionThenDirectSubmissionIsSupported) {
    auto &hwInfo = *this->rootDeviceEnvironment->getHardwareInfo();
    auto &productHelper = this->rootDeviceEnvironment->getHelper<ProductHelper>();
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_EQ(productHelper.isDirectSubmissionSupported(hwInfo), osContext->isDirectSubmissionSupported(hwInfo));
}

TEST_F(OsContextWinTest, givenWddm23ThenDirectSubmissionIsNotSupported) {
    struct WddmMock : public Wddm {
        using Wddm::featureTable;
    };
    auto wddm = static_cast<WddmMock *>(osInterface->getDriverModel()->as<Wddm>());
    wddm->featureTable.get()->flags.ftrWddmHwQueues = 1;
    osContext = std::make_unique<OsContextWin>(*wddm, 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_FALSE(osContext->isDirectSubmissionSupported(*this->rootDeviceEnvironment->getHardwareInfo()));
}

struct OsContextWinTestNoCleanup : public WddmTestWithMockGdiDllNoCleanup {
    void SetUp() override {
        WddmTestWithMockGdiDllNoCleanup::SetUp();
        preemptionMode = PreemptionHelper::getDefaultPreemptionMode(*defaultHwInfo);

        auto &gfxCoreHelper = this->rootDeviceEnvironment->getHelper<GfxCoreHelper>();
        engineTypeUsage = gfxCoreHelper.getGpgpuEngineInstances(*rootDeviceEnvironment)[0];

        init();
    }

    PreemptionMode preemptionMode;
    EngineTypeUsage engineTypeUsage;
};

TEST_F(OsContextWinTestNoCleanup, givenReinitializeContextWhenContextIsInitThenContextIsNotDestroyed) {
    osContext = std::make_unique<OsContextWin>(*osInterface->getDriverModel()->as<Wddm>(), 0, 0u, EngineDescriptorHelper::getDefaultDescriptor(engineTypeUsage, preemptionMode));
    EXPECT_FALSE(this->wddm->isDriverAvailable());
    EXPECT_TRUE(this->wddm->skipResourceCleanup());
    EXPECT_NO_THROW(osContext->reInitializeContext());
    EXPECT_NO_THROW(osContext->ensureContextInitialized());
}
