#include "vk.h"

#include <stb/stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "instance.h"
#include "swapchain.h"

ECS_DECLARE(VulkanSystem);

ECS_DECLARE(VulkanInstance);
ECS_COMPONENT_DECLARE(VkInstance);
ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);

ECS_DECLARE(PhysicalDevice);
ECS_COMPONENT_DECLARE(VkPhysicalDevice);
ECS_COMPONENT_DECLARE(SelectedPhysicalDevice);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceProperties);
ECS_COMPONENT_DECLARE(VkExtensionPropertiesArr);
ECS_COMPONENT_DECLARE(VkQueueFamilyPropertiesArr);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceFeatures);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceMemoryProperties);

ECS_DECLARE(VulkanRenderDevice);
ECS_COMPONENT_DECLARE(VkSurfaceKHR);
ECS_COMPONENT_DECLARE(VkDevice);
ECS_COMPONENT_DECLARE(VkQueue);
ECS_DECLARE(QueueIsGraphics);
ECS_COMPONENT_DECLARE(VkImageViewArr);
ECS_COMPONENT_DECLARE(VkFramebuffer);
ECS_COMPONENT_DECLARE(VkSwapchainKHR);

ECS_COMPONENT_DECLARE(VkPipeline);
ECS_COMPONENT_DECLARE(VkComputePipelineCreateInfo);
ECS_COMPONENT_DECLARE(VkGraphicsPipelineCreateInfo);
ECS_COMPONENT_DECLARE(VkRenderPass);
ECS_COMPONENT_DECLARE(VkDescriptorSetLayout);
ECS_COMPONENT_DECLARE(VkShaderModule);

ECS_COMPONENT_DECLARE(VkCommandPool);

void registerVulkan(ecs_world_t* ecs)
{
    ECS_TAG_DEFINE(ecs, VulkanSystem);
    ECS_PREFAB_DEFINE(ecs, VulkanSystem); // Slots added at the end

    ECS_TAG_DEFINE(ecs, VulkanInstance);
    ECS_COMPONENT_DEFINE(ecs, VkInstance);
    ECS_COMPONENT_DEFINE(ecs, VkDebugUtilsMessengerEXT);
    ECS_PREFAB_DEFINE(ecs, VulkanInstance, VkInstance, VkDebugUtilsMessengerEXT);

    ECS_TAG_DEFINE(ecs, PhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceProperties);
    ECS_COMPONENT_DEFINE(ecs, VkExtensionPropertiesArr);
    ECS_COMPONENT_DEFINE(ecs, VkQueueFamilyPropertiesArr);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceFeatures);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceMemoryProperties);
    ECS_PREFAB_DEFINE(ecs, PhysicalDevice,
        VkPhysicalDevice, VkPhysicalDeviceProperties, VkExtensionPropertiesArr,
        VkQueueFamilyPropertiesArr, VkPhysicalDeviceFeatures,
        VkPhysicalDeviceMemoryProperties);

    ECS_TAG_DEFINE(ecs, VulkanRenderDevice);
    ECS_COMPONENT_DEFINE(ecs, VkSurfaceKHR);
    ECS_COMPONENT_DEFINE(ecs, SelectedPhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkDevice);
    ECS_COMPONENT_DEFINE(ecs, VkQueue);
    ECS_TAG_DEFINE(ecs, QueueIsGraphics);
    ECS_COMPONENT_DEFINE(ecs, VkImageViewArr);
    ECS_COMPONENT_DEFINE(ecs, VkSwapchainKHR);
    ECS_PREFAB_DEFINE(ecs, VulkanRenderDevice,
        VkSurfaceKHR, SelectedPhysicalDevice, VkDevice, VkQueue, VkImageViewArr);

    ECS_COMPONENT_DEFINE(ecs, VkFramebuffer);
    ECS_COMPONENT_DEFINE(ecs, VkPipeline);

    ECS_COMPONENT_DEFINE(ecs, VkRenderPass);
    ECS_COMPONENT_DEFINE(ecs, VkDescriptorSetLayout);
    ECS_COMPONENT_DEFINE(ecs, VkShaderModule);

    ECS_COMPONENT_DEFINE(ecs, VkCommandPool);
    // TODO: define slots
    ecs_add_pair(ecs, VulkanInstance, EcsSlotOf, VulkanSystem);
    ecs_add_pair(ecs, VulkanInstance, EcsChildOf, VulkanSystem);
    ecs_add_pair(ecs, VulkanRenderDevice, EcsSlotOf, VulkanSystem);
    ecs_add_pair(ecs, VulkanRenderDevice, EcsChildOf, VulkanSystem);
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