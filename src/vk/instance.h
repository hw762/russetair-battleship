#pragma once

#include <vulkan/vulkan.h>

typedef struct Instance {
    VkInstance vkInstance;
} Instance;

typedef struct InstanceCreationInfo {
    uint32_t extensionCount;
    const char** ppExtensionNames;
} InstanceCreationInfo;

void createInstance(const InstanceCreationInfo* pCreationInfo, Instance* pInstance);
VkDebugUtilsMessengerEXT newVkDebugUtilsMessengerEXT(VkInstance instance);

