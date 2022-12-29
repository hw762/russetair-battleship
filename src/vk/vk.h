#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

#define vkCheck(stmt) if ((stmt) != VK_SUCCESS)

extern ECS_DECLARE(VulkanSystem);

// Instance
extern ECS_COMPONENT_DECLARE(VkInstance);
extern ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);

// Physical device
typedef VkPhysicalDevice SelectedPhysicalDevice;
typedef VkExtensionProperties* VkExtensionPropertiesArr;
typedef VkQueueFamilyProperties* VkQueueFamilyPropertiesArr;

extern ECS_COMPONENT_DECLARE(VkPhysicalDevice);
extern ECS_COMPONENT_DECLARE(SelectedPhysicalDevice);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceProperties);
extern ECS_COMPONENT_DECLARE(VkExtensionPropertiesArr);
extern ECS_COMPONENT_DECLARE(VkQueueFamilyPropertiesArr);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceFeatures);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceMemoryProperties);

// Logical device
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
/// @return Vulkan system
ecs_entity_t createVulkanSystem(ecs_world_t* ecs,
    const char** extensions, uint32_t n_extensions);