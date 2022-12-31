#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "instance.h"
#include "physical_device.h"
#include "swapchain.h"

#define vkCheck(stmt) if ((stmt) != VK_SUCCESS)

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;
struct RenderDevice;
typedef struct RenderDevice RenderDevice;

typedef struct VulkanSystem {
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
    PhysicalDevice* arrPhysicalDevices;
    RenderDevice renderDevice;
} VulkanSystem;

extern ECS_COMPONENT_DECLARE(VulkanSystem);

void registerVulkan(ecs_world_t* ecs);

VulkanSystem newVulkanSystem(const char** exts, uint32_t n_exts);
