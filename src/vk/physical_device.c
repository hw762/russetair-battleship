#include "physical_device.h"
#include "vk.h"

#include <stb_ds.h>

/// @brief Name of the `VkPhysicalDeviceType` enum.
static const char* PHYSICAL_DEVICE_TYPES[] = {
    "Other",
    "Integrated",
    "Discrete",
    "Virtual",
    "CPU"
};

/// @brief Gets all physical devices from an instnace
/// @param instance
/// @return An stb array of all devices
static VkPhysicalDevice* _getPhysicalDevices(VkInstance instance)
{
    uint32_t count;
    vkCheck(vkEnumeratePhysicalDevices(instance, &count, NULL))
    {
        ecs_abort(1, "Failed to enumerate number of physical devices");
    }
    ecs_trace("Found [%d] physical devices", count);
    VkPhysicalDevice* p = NULL;
    arrsetlen(p, count);
    vkCheck(vkEnumeratePhysicalDevices(instance, &count, p))
    {
        ecs_abort(1, "Failed to enumerate physical devices");
    }
    return p;
}

static VkPhysicalDeviceProperties
_getPhysicalDeviceProperties(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    return props;
}

static VkExtensionProperties*
_getPhysicalDeviceExtensionProperties(VkPhysicalDevice device)
{
    uint32_t n_exts;
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, NULL))
    {
        ecs_abort(1, "Failed to get number of device extension properties");
    }
    VkExtensionProperties* exts = NULL;
    arrsetlen(exts, n_exts);
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, exts))
    {
        ecs_abort(1, "Failed to get device extension properties");
    }
    return exts;
}

static VkQueueFamilyProperties*
_getPhysicalDeviceQueueFamiliyProperties(VkPhysicalDevice device)
{
    uint32_t n_qf_props;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, NULL);
    VkQueueFamilyProperties* props = NULL;
    arrsetlen(props, n_qf_props);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, props);
    return props;
}

static VkPhysicalDeviceFeatures
_getPhysicalDeviceFeatures(VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    return features;
}

static VkPhysicalDeviceMemoryProperties
_getPhysicalDeviceMemoryProperties(VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);
    return memory;
}

bool hasKHRSwapchainExt(const PhysicalDevice* phys)
{
    for (int i = 0; i < arrlen(phys->arrExtProps); ++i) {
        if (strcmp(phys->arrExtProps[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            return true;
        }
    }
    return false;
}

bool hasGraphicsQueueFamily(const PhysicalDevice* phys)
{
    const VkQueueFamilyProperties* qfs = phys->arrQueueFamilyProps;
    for (int i = 0; i < arrlen(qfs); ++i) {
        if (qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return true;
        }
    }
    return false;
}

PhysicalDevice* selectPhysicalDevice(PhysicalDevice* arrPhysicalDevices)
{
    PhysicalDevice* selected = NULL;
    ecs_trace("Selecting first appropriate device");
    ecs_log_push();
    for (int i = 0; i < arrlen(arrPhysicalDevices); ++i) {
        PhysicalDevice* phys = &arrPhysicalDevices[i];
        if (hasKHRSwapchainExt(phys) && hasGraphicsQueueFamily(phys)) {
            ecs_trace("SELECTED VkPhysicalDevice = %#p, [%s]", phys->handle, phys->props.deviceName);
            selected = phys;
            break;
        } else {
            ecs_trace("IGNORED VkPhysicalDevice = %#p, [%s]", phys->handle, phys->props.deviceName);
        }
    }
    ecs_log_pop();
    return selected;
}

////// The constructor

PhysicalDevice* getPhysicalDevices(VkInstance instance)
{
    ecs_trace("Spawning VkPhysicalDevice entities");
    ecs_log_push();

    PhysicalDevice* physicalDevices = NULL;
    VkPhysicalDevice* phys = _getPhysicalDevices(instance);
    arrsetlen(physicalDevices, arrlen(phys));
    for (int i = 0; i < arrlen(phys); ++i) {
        VkPhysicalDevice d = phys[i];
        physicalDevices[i] = (PhysicalDevice) {
            .handle = d,
            .props = _getPhysicalDeviceProperties(d),
            .arrExtProps = _getPhysicalDeviceExtensionProperties(d),
            .arrQueueFamilyProps = _getPhysicalDeviceQueueFamiliyProperties(d),
            .features = _getPhysicalDeviceFeatures(d),
            .memProps = _getPhysicalDeviceMemoryProperties(d),
        };
    }
    arrfree(phys);
    ecs_log_pop();
    return physicalDevices;
}

VkPhysicalDevice physicalDeviceHandle(const PhysicalDevice* phys)
{
    return phys->handle;
}

const VkPhysicalDeviceProperties* physicalDeviceProperties(const PhysicalDevice* phys)
{
    return &phys->props;
}

const char* physicalDeviceType(const PhysicalDevice* phys)
{
    return PHYSICAL_DEVICE_TYPES[phys->props.deviceType];
}

const VkExtensionProperties* physicalDeviceExtensionPropertiesArr(const PhysicalDevice* phys)
{
    return phys->arrExtProps;
}

const VkQueueFamilyProperties* physicalDeviceQueueFamilyPropertiesArr(const PhysicalDevice* phys)
{
    return phys->arrQueueFamilyProps;
}

const VkPhysicalDeviceFeatures* physicalDeviceFeatures(const PhysicalDevice* phys)
{
    return &phys->features;
}

const VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties(const PhysicalDevice* phys)
{
    return &phys->memProps;
}

int getGraphicsQueueFamilyIndex(const PhysicalDevice* phys)
{
    for (int i = 0; i < arrlen(phys->arrQueueFamilyProps); ++i) {
        if (phys->arrQueueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            ecs_trace("Found graphics queue family index [%d]", i);
            return i;
        }
    }
    ecs_trace("Not finding graphics queue family");
    return -1;
}

int getPresentQueueFamilyIndex(const PhysicalDevice* phys, VkSurfaceKHR surface)
{
    const VkQueueFamilyProperties* qfs = phys->arrQueueFamilyProps;
    for (int i = 0; i < arrlen(qfs); ++i) {
        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys->handle, i, surface, &supported);
        if (supported) {
            ecs_trace("Found present queue family index [%d]", i);
            return i;
        }
    }
    ecs_trace("Not finding present queue family");
    return -1;
}