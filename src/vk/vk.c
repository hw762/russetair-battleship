#include "vk.h"

#include <stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

extern const char* PROJECT_NAME;
extern const char* ENGINE_NAME;

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

void vk_register(ecs_world_t* ecs)
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

static VkInstance _vkInstance(ecs_world_t* ecs, ecs_entity_t entity, const char** extensions, uint32_t n_extensions);

ecs_entity_t vk_create_instance(ecs_world_t* ecs,
    const char** extensions, uint32_t n_extensions)
{
    ecs_trace("Creating Vulkan Instance");
    ecs_log_push();
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_add(ecs, e, VulkanInstance);
    // Create VkInstance, adding compatibility extension
    VkInstance instance = _vkInstance(ecs, e, extensions, n_extensions);
    // Set up validation layer
    ecs_log_pop();
    return e;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL _vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    (void)messageType;
    (void)pUserData;
    const char* fmt = "Validation: %s";
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ecs_err(fmt, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ecs_warn(fmt, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        ecs_trace(fmt, pCallbackData->pMessage);
        break;
    default:
        ecs_dbg(fmt, pCallbackData->pMessage);
        break;
    }
    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT _debug_utils_messenger_create_info_ext = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = _vk_debug_callback,
    .pUserData = NULL,
};

static const char** _vk_get_validation_layers()
{
    const char** layers = NULL;
    uint32_t count;
    if (vkEnumerateInstanceLayerProperties(&count, NULL) != VK_SUCCESS) {
        ecs_fatal("Failed to enumerate number of instance layer properties");
        exit(1);
    }
    VkLayerProperties* props = malloc(sizeof(*props) * count);
    if (vkEnumerateInstanceLayerProperties(&count, props) != VK_SUCCESS) {
        ecs_fatal("Failed to enumerate instance layer properties");
        exit(1);
    }
    for (uint32_t i = 0; i < count; ++i) {
        ecs_trace("Supported layer [%s]: %s", props[i].layerName, props[i].description);
    }
    for (uint32_t i = 0; i < count; ++i) {
        const char* layer = props[i].layerName;
        // Main validation layer
        if (strcmp(layer, "VK_LAYER_KHRONOS_validation") == 0) {
            arrput(layers, "VK_LAYER_KHRONOS_validation");
            break;
        }
        // Fallback 1
        if (strcmp(layer, "VK_LAYER_LUNARG_standard_validation") == 0) {
            arrput(layers, "VK_LAYER_LUNARG_standard_validation");
            break;
        }
        // Fallback 2, add as many as possible
        if (strcmp(layer, "VK_LAYER_GOOGLE_threading") == 0) {
            arrput(layers, "VK_LAYER_GOOGLE_threading");
        }
        if (strcmp(layer, "VK_LAYER_LUNARG_parameter_validation") == 0) {
            arrput(layers, "VK_LAYER_LUNARG_parameter_validation");
        }
        if (strcmp(layer, "VK_LAYER_LUNARG_object_tracker") == 0) {
            arrput(layers, "VK_LAYER_LUNARG_object_tracker");
        }
        if (strcmp(layer, "VK_LAYER_LUNARG_core_validation") == 0) {
            arrput(layers, "VK_LAYER_LUNARG_core_validation");
        }
        if (strcmp(layer, "VK_LAYER_GOOGLE_unique_objects") == 0) {
            arrput(layers, "VK_LAYER_GOOGLE_unique_objects");
        }
    }
    return layers;
}

static void
_setupDebugUtilsMessenger(ecs_world_t* ecs, ecs_entity_t entity, VkInstance instance)
{
    PFN_vkCreateDebugUtilsMessengerEXT create = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
    if (!create) {
        ecs_fatal("Failed to load debug extension");
        exit(1);
    }
    VkDebugUtilsMessengerEXT*
        messenger
        = ecs_emplace(ecs, entity, VkDebugUtilsMessengerEXT);
    if (create(instance, &_debug_utils_messenger_create_info_ext, NULL, messenger) != VK_SUCCESS) {
        ecs_fatal("Failed to create debug messenger");
        exit(1);
    }
}

static VkInstance _vkInstance(ecs_world_t* ecs, ecs_entity_t entity,
    const char** sdl_exts, uint32_t n_sdl_exts)
{
    ecs_trace("Creating VkInstance");
    ecs_log_push();
    const char** extensions = NULL;
    arrsetcap(extensions, n_sdl_exts + 2);
    memcpy(extensions, sdl_exts, sizeof(*extensions) * n_sdl_exts);
    arrput(extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    arrput(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    VkApplicationInfo app_info
        = {
              .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
              .pApplicationName = PROJECT_NAME,
              .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
              .pEngineName = ENGINE_NAME,
              .engineVersion = VK_MAKE_VERSION(0, 1, 0),
              .apiVersion = VK_API_VERSION_1_3,
          };
    // FIXME allow turning off validation
    const char** layers = _vk_get_validation_layers();
    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = arrlenu(extensions),
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = arrlenu(layers),
        .ppEnabledLayerNames = layers,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pNext = &_debug_utils_messenger_create_info_ext
    };
    VkInstance* pInstance = ecs_emplace(ecs, entity, VkInstance);
    VkResult res = vkCreateInstance(&ci, NULL, pInstance);
    if (res != VK_SUCCESS) {
        ecs_fatal("Failed to created Vulkan instance: %d", res);
        exit(1);
    }
    arrfree(extensions);
    arrfree(layers);
    _setupDebugUtilsMessenger(ecs, entity, *pInstance);
    ecs_trace("Done creating VkInstance");
    ecs_log_pop();
    return *pInstance;
}
