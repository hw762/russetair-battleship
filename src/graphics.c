#include "graphics.h"

#include <vulkan/vulkan.h>

/// @brief Declare a Vulkan object as ECS component, along with its creation info.
#define VK_OBJECT_DECLARE(id)  \
    ECS_COMPONENT_DECLARE(id); \
    ECS_COMPONENT_DECLARE(id##CreateInfo)
/// @brief Like `VK_OBJECT_DECLARE`, but applies extension suffix after id.
#define VK_OBJECT_DECLARE_EXT(id, ext) \
    ECS_COMPONENT_DECLARE(id##ext);    \
    ECS_COMPONENT_DECLARE(id##CreateInfo##ext)

/// @brief Register Vulkan object component
#define VK_OBJECT_DEFINE(ecs, id)  \
    ECS_COMPONENT_DEFINE(ecs, id); \
    ECS_COMPONENT_DEFINE(ecs, id##CreateInfo)
#define VK_OBJECT_DEFINE_EXT(ecs, id, ext) \
    ECS_COMPONENT_DEFINE(ecs, id##ext);    \
    ECS_COMPONENT_DEFINE(ecs, id##CreateInfo##ext)

// Vulkan objects
VK_OBJECT_DECLARE(VkInstance);
ECS_COMPONENT_DECLARE(VkPhysicalDevice);
VK_OBJECT_DECLARE(VkDevice);
ECS_COMPONENT_DECLARE(VkQueue);

ECS_COMPONENT_DECLARE(VkSurfaceKHR);
VK_OBJECT_DECLARE(VkImageView);
VK_OBJECT_DECLARE(VkFramebuffer);

ECS_COMPONENT_DECLARE(VkPipeline);
ECS_COMPONENT_DECLARE(VkComputePipelineCreateInfo);
ECS_COMPONENT_DECLARE(VkGraphicsPipelineCreateInfo);
VK_OBJECT_DECLARE(VkRenderPass);
VK_OBJECT_DECLARE(VkDescriptorSetLayout);
VK_OBJECT_DECLARE(VkShaderModule);

VK_OBJECT_DECLARE(VkCommandPool);

VK_OBJECT_DECLARE_EXT(VkSwapchain, KHR);

void graphics_register(ecs_world_t* ecs)
{
    VK_OBJECT_DEFINE(ecs, VkInstance);
    ECS_COMPONENT_DEFINE(ecs, VkPhysicalDevice);
    VK_OBJECT_DEFINE(ecs, VkDevice);
    ECS_COMPONENT_DEFINE(ecs, VkQueue);

    ECS_COMPONENT_DEFINE(ecs, VkSurfaceKHR);
    VK_OBJECT_DEFINE(ecs, VkImageView);
    VK_OBJECT_DEFINE(ecs, VkFramebuffer);

    ECS_COMPONENT_DEFINE(ecs, VkPipeline);
    ECS_COMPONENT_DEFINE(ecs, VkComputePipelineCreateInfo);
    ECS_COMPONENT_DEFINE(ecs, VkGraphicsPipelineCreateInfo);
    VK_OBJECT_DEFINE(ecs, VkRenderPass);
    VK_OBJECT_DEFINE(ecs, VkDescriptorSetLayout);
    VK_OBJECT_DEFINE(ecs, VkShaderModule);

    VK_OBJECT_DEFINE(ecs, VkCommandPool);

    VK_OBJECT_DEFINE_EXT(ecs, VkSwapchain, KHR);
}
