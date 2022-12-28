#include "vk.h"

#include <stdlib.h>
#include <vulkan/vulkan.h>

extern const char* PROJECT_NAME;
extern const char* ENGINE_NAME;

ECS_DECLARE(VulkanInstance);
ECS_COMPONENT_DECLARE(VkInstance);
ECS_COMPONENT_DECLARE(VkPhysicalDevice);
ECS_COMPONENT_DECLARE(VkDevice);
ECS_COMPONENT_DECLARE(VkQueue);

ECS_COMPONENT_DECLARE(VkSurfaceKHR);
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

void vk_register(ecs_world_t* ecs)
{
    ECS_TAG_DEFINE(ecs, VulkanInstance);
    ECS_COMPONENT_DEFINE(ecs, VkInstance);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDevice);
    ECS_COMPONENT_DEFINE(ecs, VkDevice);
    ECS_COMPONENT_DEFINE(ecs, VkQueue);

    ECS_COMPONENT_DEFINE(ecs, VkSurfaceKHR);
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

ecs_entity_t vk_create_instance(ecs_world_t* ecs,
    const char** extensions, uint32_t n_extensions)
{
    ecs_trace("Creating Vulkan Instance");
    ecs_log_push();
    ecs_entity_t e = ecs_new_id(ecs);
    // ECS_TAG(ecs, VulkanInstance);
    ecs_add(ecs, e, VulkanInstance);
    // Create VkInstance
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = ENGINE_NAME,
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };
    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = n_extensions,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
    };
    VkInstance* instance = ecs_emplace(ecs, e, VkInstance);
    VkResult res = vkCreateInstance(&ci, NULL, instance);
    if (res != VK_SUCCESS) {
        ecs_fatal("Failed to created Vulkan instance: %d", res);
        exit(1);
    }
    ecs_trace("Done creating VkInstance");
    // Find physical devices
finish:
    ecs_log_pop();
    return e;
}