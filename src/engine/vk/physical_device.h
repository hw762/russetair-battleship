#pragma once

#include "vulkan/vulkan_core.h"
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct Instance Instance;

typedef struct PhysicalDevice {
    VkPhysicalDevice vkPhysicalDevice;
    VkPhysicalDeviceProperties props;
    const VkExtensionProperties* arrExtProps;
    const VkQueueFamilyProperties* arrQueueFamilyProps;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceImagelessFramebufferFeatures imageless;
    VkPhysicalDeviceDynamicRenderingFeatures dynamic;
    VkPhysicalDeviceMemoryProperties memProps;
    //
    const Instance* pInstance;
} PhysicalDevice;

PhysicalDevice* getPhysicalDevices(const Instance* pInstance);

const char*
physicalDeviceType(const PhysicalDevice* phys);

bool hasKHRSwapchainExt(const PhysicalDevice* phys);
bool hasGraphicsQueueFamily(const PhysicalDevice* phys);
const PhysicalDevice* selectPhysicalDevice(const PhysicalDevice* arrPhysicalDevices);
int getGraphicsQueueFamilyIndex(const PhysicalDevice* phys);
int getPresentQueueFamilyIndex(const PhysicalDevice* phys, VkSurfaceKHR surface);
