#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

VkInstance _VkInstance(const char** extensions, uint32_t n_extensions);
VkDebugUtilsMessengerEXT _VkDebugUtilsMessengerEXT(VkInstance instance);