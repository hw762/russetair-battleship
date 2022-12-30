#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

/// @brief Select and add device to Vulkan system. Limitation: one physical device and one logical device only
/// @param ecs
/// @param eVulkanSystem
void setupDevice(ecs_world_t* ecs, ecs_entity_t eVulkanSystem);