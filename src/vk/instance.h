#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

void registerInstance(ecs_world_t* ecs);
VkInstance newVkInstance(const char** extensions, uint32_t n_extensions);
VkDebugUtilsMessengerEXT newVkDebugUtilsMessengerEXT(VkInstance instance);

