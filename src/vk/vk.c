#include "vk.h"

#include <stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "instance.h"
#include "pipeline.h"
#include "swapchain.h"

ECS_PREFAB_DECLARE(VulkanSystem);

void registerVulkan(ecs_world_t* ecs)
{
    // ECS_TAG_DEFINE(ecs, VulkanSystem);
    ECS_PREFAB_DEFINE(ecs, VulkanSystem); // Slots added at the end

    registerInstance(ecs);
    registerDevice(ecs);
    registerSwapchain(ecs);
    registerPipeline(ecs);

    // Slots
    // Vulkan system contains exactly one instance
    ecs_add_pair(ecs, VulkanInstance, EcsSlotOf, VulkanSystem);
    ecs_add_pair(ecs, VulkanInstance, EcsChildOf, VulkanSystem);
    // Vulkan system may have multiple physical devices
    ecs_add_pair(ecs, PhysicalDevice, EcsChildOf, VulkanSystem);
    // Vulkan system currently has one render device
    ecs_add_pair(ecs, RenderDevice, EcsSlotOf, VulkanSystem);
    ecs_add_pair(ecs, RenderDevice, EcsChildOf, VulkanSystem);
    // Vulkan system currently has one graphics pipeline
    ecs_add_pair(ecs, GraphicsPipeline, EcsSlotOf, VulkanSystem);
    ecs_add_pair(ecs, GraphicsPipeline, EcsChildOf, VulkanSystem);
}

void createVulkanInstance(ecs_world_t* ecs, ecs_entity_t eSystem,
    const char** extensions, uint32_t n_extensions)
{
    assert(ecs_has_pair(ecs, eSystem, EcsIsA, VulkanSystem));
    ecs_trace("Creating Vulkan Instance");
    ecs_log_push();
    ecs_entity_t eInstance = ecs_get_target(ecs, eSystem, VulkanInstance, 0);
    // Create VkInstance, adding compatibility extension
    VkInstance instance = newVkInstance(extensions, n_extensions);
    ecs_set_ptr(ecs, eInstance, VkInstance, &instance);
    // Setup messenger
    VkDebugUtilsMessengerEXT messenger = newVkDebugUtilsMessengerEXT(instance);
    ecs_set_ptr(ecs, eInstance, VkDebugUtilsMessengerEXT, &messenger);
    ecs_log_pop();
}

void setupVulkanSurface(ecs_world_t* ecs, ecs_entity_t eSystem, VkSurfaceKHR surface)
{
    assert(ecs_has_pair(ecs, eSystem, EcsIsA, VulkanSystem));
    ecs_trace("VkSurfaceKHR = %#p", surface);
    ecs_set_ptr(ecs, eSystem, VkSurfaceKHR, &surface);
}

