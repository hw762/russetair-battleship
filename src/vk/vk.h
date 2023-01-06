#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

#include "check.h"
#include "command_buffer.h"
#include "device.h"
#include "instance.h"
#include "physical_device.h"
#include "pipeline.h"
#include "queue.h"
#include "swapchain.h"

typedef struct VulkanInstance {
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
    PhysicalDevice* arrPhysicalDevices;
    Device renderDevice;
} VulkanInstance;

extern ECS_COMPONENT_DECLARE(VulkanInstance);

void registerVulkan(ecs_world_t* ecs);

VulkanInstance newVulkanInstance(const char** exts, uint32_t n_exts);
