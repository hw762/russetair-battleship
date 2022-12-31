#include "device.h"
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

static void
_addPhysicalDeviceProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    ecs_trace("Physical device [%s] (%#p): %s", props.deviceName, device, PHYSICAL_DEVICE_TYPES[props.deviceType]);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceProperties, &props);
}

static void
_addPhysicalDeviceExtensionProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    uint32_t n_exts;
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, NULL))
    {
        ecs_abort(1, "Failed to get number of device extension properties");
    }
    VkExtensionPropertiesArr exts = NULL;
    arrsetlen(exts, n_exts);
    vkCheck(vkEnumerateDeviceExtensionProperties(device, NULL, &n_exts, exts))
    {
        ecs_abort(1, "Failed to get device extension properties");
    }
    ecs_set_ptr(ecs, e, VkExtensionPropertiesArr, &exts);
}

static void
_addPhysicalDeviceQueueFamiliyProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    uint32_t n_qf_props;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, NULL);
    VkQueueFamilyPropertiesArr props = NULL;
    arrsetlen(props, n_qf_props);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_qf_props, props);
    ecs_set_ptr(ecs, e, VkQueueFamilyPropertiesArr, &props);
}

static void
_addPhysicalDeviceFeatures(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceFeatures, &features);
}

static void
_addPhysicalDeviceMemoryProperties(ecs_world_t* ecs, ecs_entity_t e, VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);
    ecs_set_ptr(ecs, e, VkPhysicalDeviceMemoryProperties, &memory);
}

static bool _hasKHRSwapchainExt(VkExtensionPropertiesArr exts)
{
    for (int i = 0; i < arrlen(exts); ++i) {
        if (strcmp(exts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            return true;
        }
    }
    return false;
}

static bool _hasGraphicsQueueFamily(VkQueueFamilyPropertiesArr qfs)
{
    for (int i = 0; i < arrlen(qfs); ++i) {
        if (qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return true;
        }
    }
    return false;
}

static ecs_entity_t _selectPhysicalDevice(ecs_world_t* ecs, ecs_entity_t system)
{
    ecs_trace("Selecting first appropriate device");
    assert(ecs_has_pair(ecs, system, EcsIsA, VulkanSystem));
    ecs_log_push();
    ecs_entity_t selected = 0;
    ecs_filter_t* f = ecs_filter(ecs,
        { .terms = {
              { ecs_id(VkPhysicalDevice) },
              { ecs_id(VkPhysicalDeviceProperties) },
              { ecs_id(VkExtensionPropertiesArr) },
              { ecs_id(VkQueueFamilyPropertiesArr) },
              { ecs_pair(EcsChildOf, system) },
              { ecs_pair(EcsIsA, PhysicalDevice) },
          } });
    ecs_iter_t it = ecs_filter_iter(ecs, f);
    while (ecs_filter_next(&it)) {
        ecs_entity_t physDevice = it.entities[0];
        assert(ecs_has_pair(ecs, physDevice, EcsIsA, PhysicalDevice));
        VkPhysicalDevice device = *ecs_field(&it, VkPhysicalDevice, 1);
        VkPhysicalDeviceProperties p = *ecs_field(&it, VkPhysicalDeviceProperties, 2);
        VkExtensionPropertiesArr exts = *ecs_field(&it, VkExtensionPropertiesArr, 3);
        VkQueueFamilyPropertiesArr qfs = *ecs_field(&it, VkQueueFamilyPropertiesArr, 4);
        bool hasKHRSwapchainExt = _hasKHRSwapchainExt(exts);
        bool hasGraphicsQueueFamily = _hasGraphicsQueueFamily(qfs);
        if (hasKHRSwapchainExt && hasGraphicsQueueFamily) {
            ecs_trace("SELECTED VkPhysicalDevice = %#p, [%s]", device, p.deviceName);
            ecs_iter_fini(&it);
            selected = physDevice;
            break;
        } else {
            ecs_trace("IGNORED VkPhysicalDevice = %#p, [%s]", device, p.deviceName);
        }
    }
    ecs_log_pop();
    return selected;
}

////// The constructor

static void
_createPhysicalDevices(ecs_world_t* ecs, ecs_entity_t parent,
    VkInstance instance)
{
    ecs_trace("Spawning VkPhysicalDevice entities");
    ecs_log_push();

    VkPhysicalDevice* physDevices = _getPhysicalDevices(instance);
    for (int i = 0; i < arrlen(physDevices); ++i) {
        ecs_entity_t e = ecs_new_w_pair(ecs, EcsIsA, PhysicalDevice);
        VkPhysicalDevice d = physDevices[i];
        ecs_add_pair(ecs, e, EcsChildOf, parent);
        ecs_set_ptr(ecs, e, VkPhysicalDevice, &d);
        _addPhysicalDeviceProperties(ecs, e, d);
        _addPhysicalDeviceExtensionProperties(ecs, e, d);
        _addPhysicalDeviceQueueFamiliyProperties(ecs, e, d);
        _addPhysicalDeviceFeatures(ecs, e, d);
        _addPhysicalDeviceMemoryProperties(ecs, e, d);
        assert(ecs_has_pair(ecs, e, EcsIsA, PhysicalDevice));
    }
    arrfree(physDevices);
    ecs_log_pop();
}

static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static ecs_entity_t
_setupDeviceQueue(ecs_world_t* ecs, ecs_entity_t e, VkDevice device, int queueFamilyIndex, int queueIndex)
{
    ecs_trace("Creating queue family [%d] index [%d] on device [%#p]",
        queueFamilyIndex, queueIndex, device);
    ecs_log_push();
    VkQueue q;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &q);
    ecs_entity_t eQueue = ecs_new_id(ecs);
    ecs_set_ptr(ecs, eQueue, VkQueue, &q);
    ecs_log_pop();
    return eQueue;
}

static int
_getGraphicsQueueFamilyIndex(VkPhysicalDevice phys, VkDevice device, VkQueueFamilyPropertiesArr props)
{
    for (int i = 0; i < arrlen(props); ++i) {
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            ecs_trace("Found graphics queue family index [%d]", i);
            return i;
        }
    }
    ecs_trace("Not finding graphics queue family");
    return -1;
}

static void _setupLogicalDevice(ecs_world_t* ecs, ecs_entity_t e, ecs_entity_t ePhysicalDevice)
{
    ecs_trace("Spawning VkLogicalDevice entities");
    ecs_log_push();
    assert(ecs_has_pair(ecs, ePhysicalDevice, EcsIsA, PhysicalDevice));
    VkPhysicalDevice physDevice = *ecs_get(ecs, ePhysicalDevice, VkPhysicalDevice);
    ecs_trace("Selected physical device %#p", physDevice);
    VkPhysicalDeviceFeatures features = { 0 };
    VkQueueFamilyPropertiesArr queueProps
        = *ecs_get(ecs, ePhysicalDevice, VkQueueFamilyPropertiesArr);
    int nQueues = arrlen(queueProps);

    VkDeviceQueueCreateInfo queueCI[nQueues];

    for (int i = 0; i < nQueues; ++i) {
        VkQueueFamilyProperties prop = queueProps[i];
        uint32_t qc = prop.queueCount;
        assert(qc > 0);
        // Destroyed when returning!
        float* priorities = alloca(qc * sizeof(float));
        memset(priorities, 0, sizeof(*priorities) * qc);
        queueCI[i]
            = (VkDeviceQueueCreateInfo) {
                  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                  .queueFamilyIndex = i,
                  .pQueuePriorities = priorities,
                  .queueCount = qc,
              };
    }
    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .ppEnabledExtensionNames = _requiredExtensions,
        .enabledExtensionCount = 1,
        .pEnabledFeatures = &features,
        .queueCreateInfoCount = nQueues,
        .pQueueCreateInfos = queueCI,
    };
    VkDevice device;
    vkCheck(vkCreateDevice(physDevice, &deviceCI, NULL, &device))
    {
        ecs_abort(1, "Failed to create logical device");
    }
    ecs_trace("Done creating VkDevice = %#p", device);
    ecs_set_ptr(ecs, e, VkDevice, &device);

    ecs_log_pop();
}

void createVulkanPhysicalDevices(ecs_world_t* ecs, ecs_entity_t eVulkanSystem)
{
    assert(ecs_has_pair(ecs, eVulkanSystem, EcsIsA, VulkanSystem));
    ecs_entity_t eInstance = ecs_get_target(ecs, eVulkanSystem, VulkanInstance, 0);
    VkInstance instance = *ecs_get(ecs, eInstance, VkInstance);
    // Populate physical device
    _createPhysicalDevices(ecs, eVulkanSystem, instance);
}

void createVulkanRenderDevice(ecs_world_t* ecs, ecs_entity_t eVulkanSystem)
{
    assert(ecs_has_pair(ecs, eVulkanSystem, EcsIsA, VulkanSystem));
    ecs_trace("Creating RenderDevice");
    ecs_log_push();
    ecs_entity_t eRenderDevice = ecs_get_target(ecs, eVulkanSystem, RenderDevice, 0);
    // Choose the best one
    ecs_entity_t selectedDevice = _selectPhysicalDevice(ecs, eVulkanSystem);
    assert(ecs_has_pair(ecs, selectedDevice, EcsIsA, PhysicalDevice));
    ecs_set_ptr(ecs, eRenderDevice, SelectedPhysicalDevice, &selectedDevice);
    VkPhysicalDevice physDev = *ecs_get(ecs, selectedDevice, VkPhysicalDevice);
    // Create logical device
    _setupLogicalDevice(ecs, eRenderDevice, selectedDevice);
    VkDevice device = *ecs_get(ecs, eRenderDevice, VkDevice);
    VkQueueFamilyPropertiesArr props = *ecs_get(ecs, selectedDevice, VkQueueFamilyPropertiesArr);
    // Create graphics queue
    int graphicsQueueFamilyIndex
        = _getGraphicsQueueFamilyIndex(physDev, device, props);
    ecs_entity_t graphicsQueue = _setupDeviceQueue(ecs, eRenderDevice, device, graphicsQueueFamilyIndex, 0);
    ecs_add(ecs, graphicsQueue, QueueIsGraphics);
    ecs_log_pop();
}