#pragma once

#include <flecs.h>

#define vkCheck(stmt) if ((stmt) != VK_SUCCESS)

// Instance
extern ECS_COMPONENT_DECLARE(VkInstance);
extern ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);

// Device
extern ECS_COMPONENT_DECLARE(VkPhysicalDevice);
extern ECS_COMPONENT_DECLARE(VkDevice);
extern ECS_COMPONENT_DECLARE(VkQueue);

// Surface
extern ECS_COMPONENT_DECLARE(VkSurfaceKHR);
extern ECS_COMPONENT_DECLARE(VkImageView);
extern ECS_COMPONENT_DECLARE(VkFramebuffer);

// Pipeline
extern ECS_COMPONENT_DECLARE(VkPipeline);
extern ECS_COMPONENT_DECLARE(VkComputePipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(VkGraphicsPipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(VkRenderPass);
extern ECS_COMPONENT_DECLARE(VkDescriptorSetLayout);
extern ECS_COMPONENT_DECLARE(VkShaderModule);

extern ECS_COMPONENT_DECLARE(VkCommandPool);

extern ECS_COMPONENT_DECLARE(VkSwapchainKHR);

void registerVulkan(ecs_world_t* ecs);

/// @brief Create a Vulkan instance, select physical device, and create logical device.
/// @param ecs
/// @return
ecs_entity_t createVulkanInstance(ecs_world_t* ecs,
    const char** extensions, uint32_t n_extensions);