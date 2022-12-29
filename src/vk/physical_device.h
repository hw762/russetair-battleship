#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

void _spawnPhysicalDevices(ecs_world_t* ecs, ecs_entity_t parent, VkInstance instance);
/// @brief Select the best physical device for vulkan system
/// TODO select best instead of first
/// @param ecs
/// @param system
/// @return the selected physical device
ecs_entity_t _selectPhysicalDevice(ecs_world_t* ecs, ecs_entity_t system);