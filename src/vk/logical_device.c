#include "logical_device.h"
#include "vk.h"

#include <stb/stb_ds.h>

static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static ecs_entity_t
_createDeviceQueue(ecs_world_t* ecs, ecs_entity_t eDevice, int queueFamilyIndex, int queueIndex)
{
    VkDevice device = *ecs_get(ecs, eDevice, VkDevice);
    ecs_trace("Creating queue on device [%#p]", device);
    ecs_log_push();
    VkQueue q;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &q);
    ecs_entity_t eQueue = ecs_new_id(ecs);
    ecs_set_ptr(ecs, eQueue, VkQueue, &q);
    ecs_log_pop();
    return eQueue;
}

static int
_getGraphicsQueueFamilyIndex(ecs_world_t* ecs, ecs_entity_t eDevice)
{
    ecs_entity_t ePhys
        = ecs_get_target(ecs, eDevice, EcsChildOf, 0);
    VkPhysicalDevice phys = *ecs_get(ecs, ePhys, VkPhysicalDevice);
    VkDevice device = *ecs_get(ecs, eDevice, VkDevice);
    ecs_dbg("VkPhysicalDevice = %#p, VkDevice = %#p", phys, device);

    VkQueueFamilyPropertiesArr props = *ecs_get(ecs, ePhys, VkQueueFamilyPropertiesArr);
    for (int i = 0; i < arrlen(props); ++i) {
        if (props[i].queueFlags | VK_QUEUE_GRAPHICS_BIT) {
            ecs_trace("Found graphics queue family index [%d]", i);
            return i;
        }
    }
    ecs_trace("Not finding graphics queue family");
    return -1;
}

/* The constructor */

ecs_entity_t _createLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity)
{
    ecs_trace("Spawning VkLogicalDevice entities");
    ecs_log_push();

    VkPhysicalDevice physDevice = *ecs_get(ecs, physDeviceEntity, VkPhysicalDevice);
    ecs_trace("Selected physical device %#p", physDevice);
    VkPhysicalDeviceFeatures features = { 0 };
    VkQueueFamilyPropertiesArr queueProps
        = *ecs_get(ecs, physDeviceEntity, VkQueueFamilyPropertiesArr);
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
        ecs_fatal("Failed to create logical device");
        exit(1);
    }
    ecs_trace("Done creating VkDevice = %#p", device);
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set_ptr(ecs, e, VkDevice, &device);
    ecs_add_pair(ecs, e, EcsChildOf, physDeviceEntity);

    int graphicsQueueFamilyIndex = _getGraphicsQueueFamilyIndex(ecs, e);

    ecs_log_pop();

    return e;
}
