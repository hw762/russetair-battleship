#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

#include "check.h"
#include "command_buffer.h"
#include "device.h"
#include "image.h"
#include "instance.h"
#include "physical_device.h"
#include "queue.h"
#include "swapchain.h"

typedef struct VulkanSystem {
    Instance instance;
    VkDebugUtilsMessengerEXT messenger;
    PhysicalDevice* arrPhysicalDevices;
    Device renderDevice;
} VulkanSystem;

extern ECS_COMPONENT_DECLARE(VulkanSystem);

void registerVulkan(ecs_world_t* ecs);

VulkanSystem newVulkanSystem(const char** exts, uint32_t n_exts);
