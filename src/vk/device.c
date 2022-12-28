#include "device.h"
#include "vk.h"

#include <stb/stb_ds.h>

VkPhysicalDevice* _getPhysicalDevices(VkInstance instance)
{
    uint32_t count;
    vkCheck(vkEnumeratePhysicalDevices(instance, &count, NULL))
    {
        ecs_fatal("Failed to enumerate number of physical devices.");
        exit(1);
    }
    ecs_trace("Found [%d] physical devices", count);
    VkPhysicalDevice* p = NULL;
    arrsetlen(p, count);
    vkCheck(vkEnumeratePhysicalDevices(instance, &count, p))
    {
        ecs_fatal("Failed to enumerate physical devices.");
        exit(1);
    }
    return p;
}

VkPhysicalDevice _choosePhysicalDevice(VkPhysicalDevice* devices)
{
    for (int i = 0; i < arrlen(devices); ++i) {
        VkPhysicalDevice p = devices[i];
        // Properties of interest
        bool hasKHRSwapchainExtension = false;
        bool hasGraphicsQueueFamily = false;
        // Properties
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(p, &props);
        ecs_trace("Checking physical device [%d] (%s).", i, props.deviceName);
        uint32_t n_exts;
        // Extensions
        vkCheck(vkEnumerateDeviceExtensionProperties(p, NULL, &n_exts, NULL))
        {
            ecs_fatal("Failed to get number of device extension properties.");
            exit(1);
        }
        {
            VkExtensionProperties exts[n_exts];
            vkCheck(vkEnumerateDeviceExtensionProperties(p, NULL, &n_exts, exts))
            {
                ecs_fatal("Failed to get device extension properties.");
                exit(1);
            }
            for (uint32_t j = 0; j < n_exts; ++j) {
                if (strcmp(exts[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    hasKHRSwapchainExtension = true;
                }
            }
        }
        // Queue families
        uint32_t n_qf_props;
        vkGetPhysicalDeviceQueueFamilyProperties(p, &n_qf_props, NULL);
        {
            VkQueueFamilyProperties props[n_qf_props];
            vkGetPhysicalDeviceQueueFamilyProperties(p, &n_qf_props, props);
            for (uint32_t j = 0; j < n_qf_props; ++j) {
                if (props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    hasGraphicsQueueFamily = true;
                }
            }
        }
        if (hasKHRSwapchainExtension && hasGraphicsQueueFamily) {
            ecs_trace("Device [%d] satisfies requirement.", i);
            return p;
        }
        ecs_trace("Device [%s] does not support required extensions.", props.deviceName);

        // // Features
        // VkPhysicalDeviceFeatures features;
        // vkGetPhysicalDeviceFeatures(p, &features);
        // // Memory properties
        // VkPhysicalDeviceMemoryProperties memory;
        // vkGetPhysicalDeviceMemoryProperties(p, &memory);
    }
    return VK_NULL_HANDLE;
}

////// The constructor

// TODO: maybe make this an entity
VkPhysicalDevice _VkPhysicalDevice(VkInstance instance)
{
    ecs_trace("Selecting VkPhysicalDevice.");
    ecs_log_push();

    VkPhysicalDevice* physDevices = _getPhysicalDevices(instance);
    VkPhysicalDevice chosenDevice = _choosePhysicalDevice(physDevices);
    arrfree(physDevices);

    ecs_log_pop();
    return NULL; // FIXME
}