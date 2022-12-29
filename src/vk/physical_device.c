#include "physical_device.h"
#include "vk.h"

#include <stb/stb_ds.h>

/// @brief Name of the `VkPhysicalDeviceType` enum.
const char* PHYSICAL_DEVICE_TYPES[] = {
    "Other",
    "Integrated",
    "Discrete",
    "Virtual",
    "CPU"
};

/// @brief Gets all physical devices from an instnace
/// @param instance
/// @return An stb array of all devices
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

void _addPhysicalDeviceProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    ecs_trace("Physical device [%s] (%#x): %s.", props.deviceName, device, PHYSICAL_DEVICE_TYPES[props.deviceType]);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceProperties, &props);
}

void _addPhysicalDeviceExtensionProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    uint32_t n_exts;
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, NULL))
    {
        ecs_fatal("Failed to get number of device extension properties.");
        exit(1);
    }
    VkExtensionPropertiesArr exts = NULL;
    arrsetlen(exts, n_exts);
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, exts))
    {
        ecs_fatal("Failed to get device extension properties.");
        exit(1);
    }
    ecs_set_ptr(ecs, e, VkExtensionPropertiesArr, &exts);
}

void _addPhysicalDeviceQueueFamiliyProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    uint32_t n_qf_props;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, NULL);
    VkQueueFamilyPropertiesArr props = NULL;
    arrsetlen(props, n_qf_props);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, props);
    ecs_set_ptr(ecs, e, VkQueueFamilyPropertiesArr, &props);
}

void _addPhysicalDeviceFeatures(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceFeatures, &features);
}

void _addPhysicalDeviceMemoryProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceMemoryProperties, &memory);
}

bool _hasKHRSwapchainExt(VkExtensionPropertiesArr exts)
{
    for (int i = 0; i < arrlen(exts); ++i) {
        if (strcmp(exts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            return true;
        }
    }
    return false;
}

bool _hasGraphicsQueueFamily(VkQueueFamilyPropertiesArr qfs)
{
    for (int i = 0; i < arrlen(qfs); ++i) {
        if (qfs[i].queueFlags | VK_QUEUE_GRAPHICS_BIT) {
            return true;
        }
    }
    return false;
}

ecs_entity_t _selectPhysicalDevice(ecs_world_t* ecs, ecs_entity_t system)
{
    ecs_entity_t selected = 0;
    ecs_trace("Selecting first appropriate device.");
    ecs_log_push();
    ecs_filter_t* f = ecs_filter(ecs,
        { .terms = {
              { ecs_id(VkPhysicalDevice) },
              { ecs_id(VkPhysicalDeviceProperties) },
              { ecs_id(VkExtensionPropertiesArr) },
              { ecs_id(VkQueueFamilyPropertiesArr) },
              { ecs_pair(EcsChildOf, system) },
          } });
    ecs_iter_t it = ecs_filter_iter(ecs, f);
    while (ecs_filter_next(&it)) {
        ecs_entity_t physDevice = it.entities[0];
        VkPhysicalDevice device = *ecs_field(&it, VkPhysicalDevice, 1);
        VkPhysicalDeviceProperties p = *ecs_field(&it, VkPhysicalDeviceProperties, 2);
        VkExtensionPropertiesArr exts = *ecs_field(&it, VkExtensionPropertiesArr, 3);
        VkQueueFamilyPropertiesArr qfs = *ecs_field(&it, VkQueueFamilyPropertiesArr, 4);
        bool hasKHRSwapchainExt = _hasKHRSwapchainExt(exts);
        bool hasGraphicsQueueFamily = _hasGraphicsQueueFamily(qfs);
        if (hasKHRSwapchainExt && hasGraphicsQueueFamily) {
            ecs_trace("SELECTED VkPhysicalDevice = %#x, [%s]", device, p.deviceName);
            ecs_iter_fini(&it);
            ecs_set_ptr(ecs, system, SelectedPhysicalDevice, &device);
            selected = physDevice;
            break;
        } else {
            ecs_trace("IGNORED VkPhysicalDevice = %#x, [%s]", device, p.deviceName);
        }
    }
    ecs_log_pop();
    return selected;
}

////// The constructor

void _spawnPhysicalDevices(ecs_world_t* ecs, ecs_entity_t system,
    VkInstance instance)
{
    ecs_trace("Spawning VkPhysicalDevice entities.");
    ecs_log_push();

    VkPhysicalDevice* physDevices = _getPhysicalDevices(instance);
    const ecs_entity_t* es = ecs_bulk_new(ecs, VkPhysicalDevice, arrlen(physDevices));
    for (int i = 0; i < arrlen(physDevices); ++i) {
        ecs_entity_t e = es[i];
        VkPhysicalDevice d = physDevices[i];
        ecs_add_pair(ecs, es[i], EcsChildOf, system);
        _addPhysicalDeviceProperties(ecs, e, d);
        _addPhysicalDeviceExtensionProperties(ecs, e, d);
        _addPhysicalDeviceQueueFamiliyProperties(ecs, e, d);
        _addPhysicalDeviceFeatures(ecs, e, d);
        _addPhysicalDeviceMemoryProperties(ecs, e, d);
    }
    arrfree(physDevices);
    ecs_log_pop();
}