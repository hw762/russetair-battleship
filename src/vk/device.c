#include "device.h"
#include "vk.h"

#include <stb_ds.h>

#include "physical_device.h"

static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

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
        .enabledExtensionCount = 1,
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

RenderDevice newRenderDevice(PhysicalDevice* arrPhysicalDevices)
{
    ecs_trace("Creating RenderDevice");
    ecs_log_push();
    // Choose the best one
    PhysicalDevice* physDev = selectPhysicalDevice(arrPhysicalDevices);
    // Create logical device
    VkDevice device = _newLogicalDevice(physDev);
    // Create graphics queue
    int graphicsQueueFamilyIndex = getGraphicsQueueFamilyIndex(physDev);
    VkQueue graphicsQueue = _newDeviceQueue(device, graphicsQueueFamilyIndex, 0);
    ecs_log_pop();
    return (RenderDevice) {
        .handle = device,
        .phys = physDev,
        .queue = graphicsQueue,
        .queueFamilyIndex = graphicsQueueFamilyIndex,
    };
}