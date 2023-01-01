#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct PhysicalDevice {
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties props;
    const VkExtensionProperties* arrExtProps;
    const VkQueueFamilyProperties* arrQueueFamilyProps;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memProps;
} PhysicalDevice;

PhysicalDevice* getPhysicalDevices(VkInstance instance);

const char*
physicalDeviceType(const PhysicalDevice* phys);

bool hasKHRSwapchainExt(const PhysicalDevice* phys);
bool hasGraphicsQueueFamily(const PhysicalDevice* phys);
PhysicalDevice* selectPhysicalDevice(PhysicalDevice* arrPhysicalDevices);
int getGraphicsQueueFamilyIndex(const PhysicalDevice* phys);
int getPresentQueueFamilyIndex(const PhysicalDevice* phys, VkSurfaceKHR surface);
