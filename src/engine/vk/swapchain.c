#include "swapchain.h"
#include "device.h"
#include "image.h"
#include "physical_device.h"
#include "vk.h"
#include "engine/utils/math.h"

#include <stb_ds.h>

static VkSurfaceCapabilitiesKHR
_getSurfaceCapabilitiesKHR(VkPhysicalDevice phys, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &capabilities))
    {
        ecs_abort(1, "Failed to get physical devce surface capabilities");
    }
    return capabilities;
}

static int _calcNumImages(VkSurfaceCapabilitiesKHR capabilities, int requestedImages)
{
    int maxImages = capabilities.maxImageCount;
    int minImages = capabilities.minImageCount;
    int result = minImages;
    if (maxImages != 0) {
        result = i32min(requestedImages, maxImages);
    }
    result = i32max(result, minImages);
    ecs_trace("Requested [%d] images, got [%d] images", requestedImages, result);
    ecs_trace("Surface capabilities, maxImages: [%d], minImages: [%d]", maxImages, minImages);
    return result;
}

static SurfaceFormat
_calcSurfaceFormat(VkPhysicalDevice phys, VkSurfaceKHR surface)
{
    uint32_t count;
    vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &count, NULL))
    {
        ecs_abort(1, "Failed to get count of surface formats");
    }
    if (count == 0) {
        ecs_abort(1, "No surface format retrieved");
    }
    VkSurfaceFormatKHR formats[count];
    vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &count, formats))
    {
        ecs_abort(1, "Failed to get surface formats");
    }
    SurfaceFormat format = { .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = formats[0].colorSpace };
    for (uint32_t i = 0; i < count; ++i) {
        VkSurfaceFormatKHR f = formats[i];
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB
            && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format.imageFormat = f.format;
            format.colorSpace = f.colorSpace;
            break;
        }
    }
    return format;
}

static VkExtent2D
_calcSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities, uint32_t defaultWidth, uint32_t defaultHeight)
{
    VkExtent2D extent;
    if (capabilities.currentExtent.width == 0xFFFFFFFF) {
        // If current extent is not defined, use default
        extent.width = u32clamp(defaultWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = u32clamp(defaultHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    } else {
        // Otherwise use current extent
        extent = capabilities.currentExtent;
    }
    return extent;
}

typedef struct {
    int aspectMask;
    int baseArrayLayer;
    int format;
    int layerCount;
    int mipLevels;
    int viewType;
} ImageViewData;

static ImageViewData
_defaultImageViewCreateInfo()
{
    return (ImageViewData) {
        .baseArrayLayer = 0,
        .layerCount = 1,
        .mipLevels = 1,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };
}

static VkImageView
_VkImageView(VkDevice device, VkImage image, ImageViewData data)
{
    VkImageViewCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = data.viewType,
        .format = data.format,
        .subresourceRange = (VkImageSubresourceRange) {
            .aspectMask = data.aspectMask,
            .baseMipLevel = 0,
            .levelCount = data.mipLevels,
            .baseArrayLayer = data.baseArrayLayer,
            .layerCount = data.layerCount,
        },
    };
    VkImageView view;
    vkCheck(vkCreateImageView(device, &ci, NULL, &view))
    {
        ecs_abort(1, "Failed to create image view");
    }
    return view;
}

static ImageView*
_newImageViews(VkDevice device, VkSwapchainKHR swapchain, int format)
{
    uint32_t count;
    VkImage* images = NULL;
    ImageView* views = NULL;

    vkCheck(vkGetSwapchainImagesKHR(device, swapchain, &count, NULL))
    {
        ecs_abort(1, "Failed to get number of surface images");
    }
    arrsetlen(images, count);
    arrsetlen(views, count);
    vkCheck(vkGetSwapchainImagesKHR(device, swapchain, &count, images))
    {
        ecs_abort(1, "Failed to get surface images");
    }
    ImageViewData data = _defaultImageViewCreateInfo();
    data.format = format;
    data.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    for (uint32_t i = 0; i < count; ++i) {
        VkFenceCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        VkFence fence;
        vkCheck(vkCreateFence(device, &ci, NULL, &fence))
        {
            ecs_abort(1, "Failed to create fence");
        }
        VkSemaphoreCreateInfo semaphoreCI = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VkSemaphore imgAcquisitionSemaphore, renderCompleteSemaphore;
        vkCheck(vkCreateSemaphore(device, &semaphoreCI, NULL, &imgAcquisitionSemaphore))
        {
            ecs_abort(1, "Failed to create image acquisition semaphore");
        }
        vkCheck(vkCreateSemaphore(device, &semaphoreCI, NULL, &renderCompleteSemaphore))
        {
            ecs_abort(1, "Failed to create render complete semaphore");
        }
        VkImageView imgView = _VkImageView(device, images[i], data);
        views[i]
            = (ImageView) {
                  .handle = imgView,
                  .image = images[i],
                  .fence = fence,
                  .acquisitionSemaphore = imgAcquisitionSemaphore,
                  .renderCompleteSemaphore = renderCompleteSemaphore
              };
        ecs_trace("Created VkImageView = %#p on VkSwapchainKHR = %#p", imgView, swapchain);
    }
    return views;
}

void createSwapchain(const SwapchainCreateInfo* pCreateInfo, Swapchain* pSwapchain)
{
    const Device* renderDevice = pCreateInfo->pDevice;
    const PhysicalDevice* physDev = renderDevice->phys;
    VkPhysicalDevice vkPhysDev = physDev->vkPhysicalDevice;
    VkDevice device = renderDevice->vkDevice;
    ecs_trace("Creating Swapchain on device [%#p]", device);
    ecs_log_push();

    VkSurfaceCapabilitiesKHR capabilities = _getSurfaceCapabilitiesKHR(vkPhysDev, pCreateInfo->surface);
    int numImages = _calcNumImages(capabilities, pCreateInfo->requestedImages);
    SurfaceFormat format = _calcSurfaceFormat(vkPhysDev, pCreateInfo->surface);
    VkExtent2D extent = _calcSwapchainExtent(capabilities, pCreateInfo->defaultWidth, pCreateInfo->defaultHeight);

    VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = pCreateInfo->surface,
        .minImageCount = numImages,
        .imageFormat = format.imageFormat,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .clipped = true,
    };
    if (pCreateInfo->vSync) {
        ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    } else {
        ci.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    VkSwapchainKHR swapchain;
    vkCheck(vkCreateSwapchainKHR(device, &ci, NULL, &swapchain))
    {
        ecs_abort(1, "Failed to create swapchain");
    }
    ecs_trace("VkSwapchainKHR = %#p", swapchain);

    ImageView* views = _newImageViews(device, swapchain, format.imageFormat);

    ecs_log_pop();
    *pSwapchain = (Swapchain) {
        .handle = swapchain,
        .device = renderDevice,
        .surface = pCreateInfo->surface,
        .arrViews = views,
        .format = format,
        .currentFrame = 0,
        .extent = extent,
    };
}

const ImageView* swapchainCurrentView(const Swapchain* swapchain)
{
    return &swapchain->arrViews[swapchain->currentFrame];
}

bool swapchainAcquire(Swapchain* swapchain)
{
    bool resize = false;
    const ImageView* view = swapchainCurrentView(swapchain);
    switch (vkAcquireNextImageKHR(swapchain->device->vkDevice, swapchain->handle, -0L,
        view->acquisitionSemaphore, NULL, &swapchain->currentFrame)) {
    case VK_ERROR_OUT_OF_DATE_KHR:
        resize = true;
        break;
    case VK_SUBOPTIMAL_KHR:
        resize = true;
        break;
    case VK_SUCCESS:
        break;
    default:
        ecs_abort(1, "Failed to acquire image")
    }
    return resize;
}

bool swapchainPresent(Swapchain* swapchain, VkQueue queue)
{
    bool resize = false;

    const ImageView* view = swapchainCurrentView(swapchain);
    ecs_trace("Presenting to image [%d]", swapchain->currentFrame);
    VkPresentInfoKHR presentInfo
        = {
              .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
              .pWaitSemaphores = &view->renderCompleteSemaphore,
              .waitSemaphoreCount = 1,
              .swapchainCount = 1,
              .pSwapchains = &swapchain->handle,
              .pImageIndices = &swapchain->currentFrame,
          };
    switch (vkQueuePresentKHR(queue, &presentInfo)) {
    case VK_ERROR_OUT_OF_DATE_KHR:
        resize = true;
        break;
    case VK_SUBOPTIMAL_KHR:
        break;
    case VK_SUCCESS:
        break;
    default:
        ecs_abort(1, "Failed to present KHR");
    }
    swapchain->currentFrame = (swapchain->currentFrame + 1) % arrlen(swapchain->arrViews);
    return resize;
}