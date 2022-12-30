#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

VkInstance newVkInstance(const char** extensions, uint32_t n_extensions);
VkDebugUtilsMessengerEXT newVkDebugUtilsMessengerEXT(VkInstance instance);