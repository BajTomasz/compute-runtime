/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/core/source/image/image_imp.h"

#include "shared/source/device/device.h"
#include "shared/source/execution_environment/execution_environment.h"
#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/helpers/bindless_heaps_helper.h"
#include "shared/source/helpers/gfx_core_helper.h"
#include "shared/source/helpers/hw_info.h"
#include "shared/source/memory_manager/memory_manager.h"

#include "level_zero/core/source/device/device.h"
#include "level_zero/core/source/device/device_imp.h"
#include "level_zero/core/source/driver/driver_handle_imp.h"

#include "igfxfmid.h"

namespace L0 {

ImageAllocatorFn imageFactory[IGFX_MAX_PRODUCT] = {};

ImageImp::~ImageImp() {
    if (isImageView() && this->device != nullptr && this->allocation) {
        if (bindlessInfo.get() && this->device->getNEODevice()->getExecutionEnvironment()->rootDeviceEnvironments[this->allocation->getRootDeviceIndex()]->getBindlessHeapsHelper() != nullptr) {
            this->device->getNEODevice()->getExecutionEnvironment()->rootDeviceEnvironments[this->allocation->getRootDeviceIndex()]->getBindlessHeapsHelper()->releaseSSToReusePool(*bindlessInfo);
        }
    }
    if (!isImageView() && this->device != nullptr) {
        this->device->getNEODevice()->getMemoryManager()->freeGraphicsMemory(this->allocation);
    }
}

ze_result_t ImageImp::destroy() {
    if (this->getAllocation() && this->device) {
        auto imageAllocPtr = reinterpret_cast<const void *>(this->getAllocation()->getGpuAddress());
        DriverHandleImp *driverHandle = static_cast<DriverHandleImp *>(this->device->getDriverHandle());

        for (auto peerDevice : driverHandle->devices) {
            this->destroyPeerImages(imageAllocPtr, peerDevice);
        }
    }

    delete this;
    return ZE_RESULT_SUCCESS;
}

ze_result_t ImageImp::destroyPeerImages(const void *ptr, Device *device) {
    DeviceImp *deviceImp = static_cast<DeviceImp *>(device);

    std::unique_lock<NEO::SpinLock> lock(deviceImp->peerImageAllocationsMutex);

    if (deviceImp->peerImageAllocations.find(ptr) != deviceImp->peerImageAllocations.end()) {
        delete deviceImp->peerImageAllocations[ptr];
        deviceImp->peerImageAllocations.erase(ptr);
    }

    return ZE_RESULT_SUCCESS;
}

ze_result_t ImageImp::createView(Device *device, const ze_image_desc_t *desc, ze_image_handle_t *pImage) {
    auto productFamily = device->getNEODevice()->getHardwareInfo().platform.eProductFamily;

    ImageAllocatorFn allocator = nullptr;
    allocator = imageFactory[productFamily];

    ImageImp *image = nullptr;

    image = static_cast<ImageImp *>((*allocator)());
    image->allocation = allocation;
    image->sourceImageFormatDesc = this->imageFormatDesc;
    auto result = image->initialize(device, desc);

    if (result != ZE_RESULT_SUCCESS) {
        image->destroy();
        image = nullptr;
    }

    *pImage = image;

    return result;
}

ze_result_t ImageImp::allocateBindlessSlot() {
    if (!isImageView()) {
        if (!this->device->getNEODevice()->getMemoryManager()->allocateBindlessSlot(allocation)) {
            return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
        }
        if (allocation->getBindlessOffset() != std::numeric_limits<uint64_t>::max()) {
            bindlessInfo = std::make_unique<NEO::SurfaceStateInHeapInfo>(allocation->getBindlessInfo());
        }
        return ZE_RESULT_SUCCESS;
    }
    auto bindlessHelper = this->device->getNEODevice()->getExecutionEnvironment()->rootDeviceEnvironments[allocation->getRootDeviceIndex()]->getBindlessHeapsHelper();

    if (bindlessHelper && !bindlessInfo) {
        auto &gfxCoreHelper = this->device->getNEODevice()->getExecutionEnvironment()->rootDeviceEnvironments[allocation->getRootDeviceIndex()]->getHelper<NEO::GfxCoreHelper>();
        const auto surfStateCount = 2;
        auto surfaceStateSize = surfStateCount * gfxCoreHelper.getRenderSurfaceStateSize();

        auto surfaceStateInfo = bindlessHelper->allocateSSInHeap(surfaceStateSize, allocation, NEO::BindlessHeapsHelper::GLOBAL_SSH);
        if (surfaceStateInfo.heapAllocation == nullptr) {
            return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
        }
        bindlessInfo = std::make_unique<NEO::SurfaceStateInHeapInfo>(surfaceStateInfo);
    }
    return ZE_RESULT_SUCCESS;
}

NEO::SurfaceStateInHeapInfo *ImageImp::getBindlessSlot() {
    return bindlessInfo.get();
}

ze_result_t Image::create(uint32_t productFamily, Device *device, const ze_image_desc_t *desc, Image **pImage) {
    ze_result_t result = ZE_RESULT_SUCCESS;
    ImageAllocatorFn allocator = nullptr;
    if (productFamily < IGFX_MAX_PRODUCT) {
        allocator = imageFactory[productFamily];
    }

    ImageImp *image = nullptr;
    if (allocator) {
        image = static_cast<ImageImp *>((*allocator)());
        result = image->initialize(device, desc);
        if (result != ZE_RESULT_SUCCESS) {
            image->destroy();
            image = nullptr;
        }
    } else {
        result = ZE_RESULT_ERROR_UNKNOWN;
    }
    *pImage = image;

    return result;
}
} // namespace L0
