#include "logical_device.h"
#include "vk.h"

#include <stb/stb_ds.h>

static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

/* The constructor */

ecs_entity_t _spawnLogicalDevice(ecs_world_t* ecs, ecs_entity_t physDeviceEntity)
{
    ecs_trace("Spawning VkLogicalDevice entities.");
    ecs_log_push();

    VkPhysicalDevice physDevice = *ecs_get(ecs, physDeviceEntity, VkPhysicalDevice);
    ecs_trace("Selected physical device %#lx.", physDevice);
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
    return e;
}
