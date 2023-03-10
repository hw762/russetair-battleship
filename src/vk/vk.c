#include "vk.h"

#include <stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "instance.h"
#include "physical_device.h"
#include "pipeline.h"
#include "swapchain.h"

ECS_COMPONENT_DECLARE(VulkanSystem);

void registerVulkan(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, VulkanSystem);
}

VulkanSystem newVulkanSystem(const char** exts, uint32_t n_exts)
{
    ecs_trace("Creating Vulkan Instance");
    ecs_log_push();
    // Create VkInstance, adding compatibility extension
    VkInstance instance = newVkInstance(exts, n_exts);
    // Setup messenger
    VkDebugUtilsMessengerEXT messenger = newVkDebugUtilsMessengerEXT(instance);
    PhysicalDevice* phys = getPhysicalDevices(instance);
    RenderDevice device = newRenderDevice(phys);
    ecs_log_pop();
    return (VulkanSystem) {
        .instance = instance,
        .messenger = messenger,
        .arrPhysicalDevices = phys,
        .renderDevice = device,
    };
}
