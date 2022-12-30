#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

/// @brief Spawn logical device and add as child to physical device
ecs_entity_t createLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity);

ecs_entity_t createDeviceQueue(ecs_world_t* ecs, ecs_entity_t eDevice, int queueFamilyIndex, int queueIndex);
int getGraphicsQueueFamilyIndex(ecs_world_t* ecs, ecs_entity_t eDevice);