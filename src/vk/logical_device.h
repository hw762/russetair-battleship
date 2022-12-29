#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

/// @brief Spawns logical device and add as child to physical device
ecs_entity_t _createLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity);
