#include "vk.h"

#include <stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

ECS_DECLARE(VulkanInstance);
ECS_COMPONENT_DECLARE(VkInstance);
ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);
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

void registerVulkan(ecs_world_t* ecs)
{
    ECS_TAG_DEFINE(ecs, VulkanInstance);
    ECS_COMPONENT_DEFINE(ecs, VkInstance);
    ECS_COMPONENT_DEFINE(ecs, VkDebugUtilsMessengerEXT);
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
