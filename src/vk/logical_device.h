#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

/// @brief Add logical device to physical device entity
void createLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity);

ecs_entity_t createDeviceQueue(ecs_world_t* ecs, ecs_entity_t eDevice, int queueFamilyIndex, int queueIndex);
int getGraphicsQueueFamilyIndex(ecs_world_t* ecs, ecs_entity_t eDevice);