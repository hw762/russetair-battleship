#include "vk.h"

#include <stb/stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "instance.h"

#include <vulkan/vulkan.h>

ECS_DECLARE(VulkanSystem);
ECS_COMPONENT_DECLARE(VkInstance);
ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);
ECS_COMPONENT_DECLARE(VkSurfaceKHR);

ECS_DECLARE(PhysicalDevice);
ECS_COMPONENT_DECLARE(VkPhysicalDevice);
ECS_COMPONENT_DECLARE(SelectedPhysicalDevice);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceProperties);
ECS_COMPONENT_DECLARE(VkExtensionPropertiesArr);
ECS_COMPONENT_DECLARE(VkQueueFamilyPropertiesArr);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceFeatures);
ECS_COMPONENT_DECLARE(VkPhysicalDeviceMemoryProperties);

ECS_DECLARE(LogicalDevice);
ECS_COMPONENT_DECLARE(VkDevice);
ECS_DECLARE(GraphicsQueue);
ECS_COMPONENT_DECLARE(VkQueue);

ECS_COMPONENT_DECLARE(VkImageView);
ECS_COMPONENT_DECLARE(VkFramebuffer);

ECS_COMPONENT_DECLARE(VkPipeline);
ECS_COMPONENT_DECLARE(VkComputePipelineCreateInfo);
ECS_COMPONENT_DECLARE(VkGraphicsPipelineCreateInfo);
ECS_COMPONENT_DECLARE(VkRenderPass);
ECS_COMPONENT_DECLARE(VkDescriptorSetLayout);
ECS_COMPONENT_DECLARE(VkShaderModule);

ECS_COMPONENT_DECLARE(VkCommandPool);

ECS_COMPONENT_DECLARE(VkSwapchainKHR);

void registerVulkan(ecs_world_t* ecs)
{
    ECS_TAG_DEFINE(ecs, VulkanSystem);
    ECS_COMPONENT_DEFINE(ecs, VkInstance);
    ECS_COMPONENT_DEFINE(ecs, VkDebugUtilsMessengerEXT);
    ECS_COMPONENT_DEFINE(ecs, VkSurfaceKHR);

    ECS_TAG_DEFINE(ecs, PhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, SelectedPhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceProperties);
    ECS_COMPONENT_DEFINE(ecs, VkExtensionPropertiesArr);
    ECS_COMPONENT_DEFINE(ecs, VkQueueFamilyPropertiesArr);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceFeatures);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDeviceMemoryProperties);

    ECS_TAG_DEFINE(ecs, LogicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkDevice);
    ECS_TAG_DEFINE(ecs, GraphicsQueue);
    ECS_COMPONENT_DEFINE(ecs, VkQueue);

    ECS_COMPONENT_DEFINE(ecs, VkImageView);
    ECS_COMPONENT_DEFINE(ecs, VkFramebuffer);

    ECS_COMPONENT_DEFINE(ecs, VkPipeline);
    ECS_COMPONENT_DEFINE(ecs, VkComputePipelineCreateInfo);
    ECS_COMPONENT_DEFINE(ecs, VkGraphicsPipelineCreateInfo);
    ECS_COMPONENT_DEFINE(ecs, VkRenderPass);
    ECS_COMPONENT_DEFINE(ecs, VkDescriptorSetLayout);
    ECS_COMPONENT_DEFINE(ecs, VkShaderModule);

    ECS_COMPONENT_DEFINE(ecs, VkCommandPool);

    ECS_COMPONENT_DEFINE(ecs, VkSwapchainKHR);
}

ecs_entity_t createVulkanInstanceAndDevices(ecs_world_t* ecs,
    const char** extensions, uint32_t n_extensions)
{
    ecs_trace("Creating Vulkan Instance");
    ecs_log_push();
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_add(ecs, e, VulkanSystem);
    // Create VkInstance, adding compatibility extension
    VkInstance instance = newVkInstance(extensions, n_extensions);
    ecs_set_ptr(ecs, e, VkInstance, &instance);
    // Setup messenger
    VkDebugUtilsMessengerEXT messenger = newVkDebugUtilsMessengerEXT(instance);
    ecs_set_ptr(ecs, e, VkDebugUtilsMessengerEXT, &messenger);
    setupDevice(ecs, e);
    ecs_log_pop();
    return e;
}

void setupVulkanSurface(ecs_world_t* ecs, ecs_entity_t eInstance, VkSurfaceKHR surface)
{
    ecs_trace("VkSurfaceKHR = %#p", surface);
    ecs_set_ptr(ecs, eInstance, VkSurfaceKHR, &surface);
}