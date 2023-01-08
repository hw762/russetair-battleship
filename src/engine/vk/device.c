#include "device.h"
#include "vk.h"

#include <limits.h>
#include <stb_ds.h>

#include "memory.h"
#include "physical_device.h"
#include "vulkan/vulkan_core.h"

#ifdef __APPLE__
static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    "VK_KHR_portability_subset",
};

static const uint32_t _nRequiredExtensions = 3;
#else
static const char* _requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
};

static const uint32_t _nRequiredExtensions = 2;
#endif

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
    VkPhysicalDevice vkPhysicalDevice = phys->vkPhysicalDevice;
    VkPhysicalDeviceFeatures features = {};
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
    VkPhysicalDeviceImagelessFramebufferFeatures imageless = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES,
        .imagelessFramebuffer = VK_TRUE,
        .pNext = &dynamic,
    };
    VkPhysicalDeviceSynchronization2Features sync2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .synchronization2 = VK_TRUE,
        .pNext = &imageless,
    };
    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .ppEnabledExtensionNames = _requiredExtensions,
        .enabledExtensionCount = _nRequiredExtensions,
        .pEnabledFeatures = &features,
        .queueCreateInfoCount = nQueues,
        .pQueueCreateInfos = queueCI,
        .pNext = &sync2,
    };
    VkDevice device;
    vkIfFailed(vkCreateDevice(vkPhysicalDevice, &deviceCI, NULL, &device))
    {
        ecs_abort(1, "Failed to create logical device");
    }
    ecs_trace("Done creating VkDevice = %#p", device);

    ecs_log_pop();
    return device;
}

void createDevice(const PhysicalDevice* arrPhysicalDevices, Device* pDevice)
{
    ecs_trace("Creating RenderDevice");
    ecs_log_push();
    // Choose the best one
    pDevice->phys = selectPhysicalDevice(arrPhysicalDevices);
    // Create logical device
    pDevice->vkDevice = _newLogicalDevice(pDevice->phys);
    createMemoryAllocator(pDevice, &pDevice->allocator);
    ecs_log_pop();
}

void destroyDevice(Device* pDevice)
{
    vkDestroyDevice(pDevice->vkDevice, NULL);
    pDevice->vkDevice = VK_NULL_HANDLE;
}

Queue deviceGetGraphicsQueue(const Device* device)
{
    ecs_trace("Creating graphics queue");
    ecs_log_push();
    int index = getGraphicsQueueFamilyIndex(device->phys);
    VkQueue q = _newDeviceQueue(device->vkDevice, index, 0);
    ecs_trace("Done creating graphics queue VkQueue = %#p on VkDevice = %#p", q, device->vkDevice);
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
    VkQueue q = _newDeviceQueue(device->vkDevice, index, 0);
    ecs_trace("Done creating present queue VkQueue = %#p on VkDevice = %#p", q, device->vkDevice);
    ecs_log_pop();
    return (Queue) {
        .handle = q,
        .device = device,
        .queueFamilyIndex = index,
    };
}
