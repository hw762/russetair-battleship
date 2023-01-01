#include "instance.h"
#include "vk.h"

#include <stb_ds.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

extern const char* PROJECT_NAME;
extern const char* ENGINE_NAME;

static VKAPI_ATTR VkBool32 VKAPI_CALL _vkDebugCallback(
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
        ecs_dbg(fmt, pCallbackData->pMessage);
        break;
    default:
        // Anything else gets even lower level
        ecs_dbg(fmt, pCallbackData->pMessage);
        break;
    }
    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT _debug_utils_messenger_create_info_ext = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = _vkDebugCallback,
    .pUserData = NULL,
};

static const char** _getValidationLayers()
{
    const char** layers = NULL;
    uint32_t count;
    if (vkEnumerateInstanceLayerProperties(&count, NULL) != VK_SUCCESS) {
        ecs_abort(1, "Failed to enumerate number of instance layer properties");
    }
    VkLayerProperties* props = malloc(sizeof(*props) * count);
    if (vkEnumerateInstanceLayerProperties(&count, props) != VK_SUCCESS) {
        ecs_abort(1, "Failed to enumerate instance layer properties");
    }
    for (uint32_t i = 0; i < count; ++i) {
        ecs_trace("Supported validation layer [%s]: %s", props[i].layerName, props[i].description);
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

static const char**
_getRequiredExtensions(const char** sdl_exts, uint32_t n_sdl_exts)
{
    const char** extensions = NULL;
    arrsetlen(extensions, n_sdl_exts);
    memcpy(extensions, sdl_exts, sizeof(*extensions) * n_sdl_exts);
#ifdef __APPLE__
    arrput(extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    arrput(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

VkDebugUtilsMessengerEXT newVkDebugUtilsMessengerEXT(VkInstance instance)
{
    ecs_trace("Setting up messenger");
    ecs_log_push();
    PFN_vkCreateDebugUtilsMessengerEXT create = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
    if (!create) {
        ecs_abort(1, "Failed to load debug extension");
    }
    VkDebugUtilsMessengerEXT messenger;
    if (create(instance, &_debug_utils_messenger_create_info_ext, NULL, &messenger) != VK_SUCCESS) {
        ecs_abort(1, "Failed to create debug messenger");
    }
    ecs_trace("Done setting up messenger");
    ecs_log_pop();
    return messenger;
}

VkInstance newVkInstance(const char** sdl_exts, uint32_t n_sdl_exts)
{
    ecs_trace("Creating VkInstance");
    ecs_log_push();
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
    const char** extensions = _getRequiredExtensions(sdl_exts, n_sdl_exts);
    const char** layers = _getValidationLayers();
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
    VkInstance instance;
    vkCheck(vkCreateInstance(&ci, NULL, &instance))
    {
        ecs_abort(1, "Failed to created Vulkan instance");
    }
    arrfree(extensions);
    arrfree(layers);
    ecs_trace("Done creating VkInstance = %#llx", instance);
    ecs_log_pop();
    return instance;
}

