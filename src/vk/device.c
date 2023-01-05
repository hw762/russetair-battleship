#include "device.h"
#include "vk.h"

#include <limits.h>
#include <stb_ds.h>

#include "physical_device.h"

static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_portability_subset",
};

static const uint32_t _nRequiredExtensions = 2;

static VkQueue
_newDeviceQueue(VkDevice device, int queueFamilyIndex, int queueIndex)
{
    ecs_trace("Creating queue family [%d] index [%d] on device [%#p]",
        queueFamilyIndex, queueIndex, device);
    VkQueue q;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &q);
    return q;
}

static VkDevice _newLogicalDevice(const PhysicalDevice* phys)
{
    ecs_trace("Creating logical device");
    ecs_log_push();
    ecs_trace("Selected physical device %#p", phys);
    VkPhysicalDevice vkPhysicalDevice = phys->handle;
    VkPhysicalDeviceFeatures features = { 0 };
    const VkQueueFamilyProperties* queueProps
        = phys->arrQueueFamilyProps;

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
    VkPhysicalDeviceDynamicRenderingFeatures dynamic = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = VK_TRUE,
    };
    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .ppEnabledExtensionNames = _requiredExtensions,
        .enabledExtensionCount = _nRequiredExtensions,
        .pEnabledFeatures = &features,
        .queueCreateInfoCount = nQueues,
        .pQueueCreateInfos = queueCI,
        .pNext = &dynamic,
    };
    VkDevice device;
    vkCheck(vkCreateDevice(vkPhysicalDevice, &deviceCI, NULL, &device))
    {
        ecs_abort(1, "Failed to create logical device");
    }
    ecs_trace("Done creating VkDevice = %#p", device);

    ecs_log_pop();
    return device;
}

Device newRenderDevice(PhysicalDevice* arrPhysicalDevices)
{
    ecs_trace("Creating RenderDevice");
    ecs_log_push();
    // Choose the best one
    PhysicalDevice* physDev = selectPhysicalDevice(arrPhysicalDevices);
    // Create logical device
    VkDevice device = _newLogicalDevice(physDev);
    ecs_log_pop();
    return (Device) {
        .handle = device,
        .phys = physDev,
    };
}

Queue deviceGetGraphicsQueue(const Device* device)
{
    ecs_trace("Creating graphics queue");
    ecs_log_push();
    int index = getGraphicsQueueFamilyIndex(device->phys);
    VkQueue q = _newDeviceQueue(device->handle, index, 0);
    ecs_trace("Done creating graphics queue VkQueue = %#p on VkDevice = %#p", q, device->handle);
    ecs_log_pop();
    return (Queue) {
        .handle = q,
        .device = device,
        .queueFamilyIndex = index,
    };
}

Queue deviceGetPresentQueue(const Device* device, VkSurfaceKHR surface)
{
    ecs_trace("Creating present queue");
    ecs_log_push();
    int index = getPresentQueueFamilyIndex(device->phys, surface);
    VkQueue q = _newDeviceQueue(device->handle, index, 0);
    ecs_trace("Done creating present queue VkQueue = %#p on VkDevice = %#p", q, device->handle);
    ecs_log_pop();
    return (Queue) {
        .handle = q,
        .device = device,
        .queueFamilyIndex = index,
    };
}
