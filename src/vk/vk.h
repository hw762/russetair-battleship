#pragma once

#include <flecs.h>
#include <vulkan/vulkan.h>

#define vkCheck(stmt) if ((stmt) != VK_SUCCESS)

extern ECS_PREFAB_DECLARE(VulkanSystem);

// Instance
extern ECS_PREFAB_DECLARE(VulkanInstance);
extern ECS_COMPONENT_DECLARE(VkInstance);
extern ECS_COMPONENT_DECLARE(VkDebugUtilsMessengerEXT);

// Physical device
typedef ecs_entity_t SelectedPhysicalDevice;
typedef VkExtensionProperties* VkExtensionPropertiesArr;
typedef VkQueueFamilyProperties* VkQueueFamilyPropertiesArr;

extern ECS_PREFAB_DECLARE(PhysicalDevice);
extern ECS_COMPONENT_DECLARE(VkPhysicalDevice);
extern ECS_COMPONENT_DECLARE(SelectedPhysicalDevice);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceProperties);
extern ECS_COMPONENT_DECLARE(VkExtensionPropertiesArr);
extern ECS_COMPONENT_DECLARE(VkQueueFamilyPropertiesArr);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceFeatures);
extern ECS_COMPONENT_DECLARE(VkPhysicalDeviceMemoryProperties);

// Logical device
extern ECS_PREFAB_DECLARE(RenderDevice);
extern ECS_COMPONENT_DECLARE(VkSurfaceKHR);
extern ECS_COMPONENT_DECLARE(VkDevice);
extern ECS_COMPONENT_DECLARE(VkQueue);
extern ECS_TAG_DECLARE(QueueIsGraphics);

// Swapchain
struct SurfaceFormat;
typedef struct SurfaceFormat SurfaceFormat;
typedef VkImageView* VkImageViewArr;
extern ECS_PREFAB_DECLARE(Swapchain);
extern ECS_COMPONENT_DECLARE(VkImageViewArr);
extern ECS_COMPONENT_DECLARE(VkSwapchainKHR);
extern ECS_COMPONENT_DECLARE(VkFramebuffer);
extern ECS_COMPONENT_DECLARE(SurfaceFormat);

// Pipeline
extern ECS_PREFAB_DECLARE(GraphicsPipeline);
extern ECS_COMPONENT_DECLARE(VkPipeline);
extern ECS_COMPONENT_DECLARE(VkRenderPass);
extern ECS_COMPONENT_DECLARE(VkDescriptorSetLayout);
typedef VkShaderModule VertexShaderModule;
typedef VkShaderModule FragmentShaderModule;
extern ECS_COMPONENT_DECLARE(VertexShaderModule);
extern ECS_COMPONENT_DECLARE(FragmentShaderModule);
extern ECS_COMPONENT_DECLARE(VkCommandPool);

void registerVulkan(ecs_world_t* ecs);

/// @brief Create a Vulkan instance in a system
/// @param ecs system
void createVulkanInstance(ecs_world_t* ecs,
    ecs_entity_t eSystem,
    const char** extensions, uint32_t n_extensions);

/// @brief Select and add device to Vulkan system. Limitation: one physical device and one logical device only
/// @param ecs
/// @param eVulkanSystem
void createVulkanPhysicalDevices(ecs_world_t* ecs, ecs_entity_t eVulkanSystem);
void createVulkanRenderDevice(ecs_world_t* ecs, ecs_entity_t eVulkanSystem);

/// @brief Sets up a surface
/// @param ecs
/// @param eInstance
/// @param surface
void setupVulkanSurface(ecs_world_t* ecs, ecs_entity_t eInstance, VkSurfaceKHR surface);

void setupSwapchain(ecs_world_t* ecs, ecs_entity_t eSystem,
    int requestedImages, bool vsync,
    uint32_t defaultWidth, uint32_t defaultHeight);

void cleanupSwapchain(ecs_world_t* ecs, ecs_entity_t eSystem);
