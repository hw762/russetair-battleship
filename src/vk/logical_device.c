#include "logical_device.h"
#include "vk.h"

#include <stb/stb_ds.h>

/* The constructor */

ecs_entity_t _spawnLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity)
{
    ecs_trace("Spawning VkLogicalDevice entities.");
    ecs_log_push();
    const char* _requiredExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VkPhysicalDevice physDevice = *ecs_get(ecs, physDeviceEntity, VkPhysicalDevice);
    ecs_trace("Selected physical device %#x.", physDevice);
    VkPhysicalDeviceFeatures features = { 0 };
    VkQueueFamilyPropertiesArr queueProps
        = *ecs_get(ecs, physDeviceEntity, VkQueueFamilyPropertiesArr);
    int nQueues = arrlen(queueProps);

    VkDeviceQueueCreateInfo queueCI[nQueues];
    float** ppPriorities = NULL;
    arrsetlen(ppPriorities, nQueues);
    for (int i = 0; i < nQueues; ++i) {
        VkQueueFamilyProperties prop = queueProps[i];
        ppPriorities[i] = NULL;
        arrsetlen(ppPriorities, prop.queueCount);
        float* priorities = ppPriorities[i];
        memset(priorities, 0, sizeof(*priorities) * prop.queueCount);
        queueCI[i]
            = (VkDeviceQueueCreateInfo) {
                  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                  .queueFamilyIndex = i,
                  .pQueuePriorities = priorities,
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
    ecs_trace("Created VkDeviceCreateInfo.");
    VkDevice device;
    vkCheck(vkCreateDevice(physDevice, &deviceCI, NULL, &device))
    {
        ecs_fatal("Failed to create logical device.");
        exit(1);
    }
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set_ptr(ecs, e, VkDevice, &device);
    ecs_add_pair(ecs, e, EcsChildOf, physDeviceEntity);
    ecs_log_pop();
    for (int i = 0; i < nQueues; ++i) {
        arrfree(ppPriorities[i]);
    }
    arrfree(ppPriorities);
    return e;
}
