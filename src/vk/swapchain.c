#include "swapchain.h"
#include "vk.h"

#include <stb_ds.h>
#include <utils/math.h>

ECS_PREFAB_DECLARE(Swapchain);
ECS_COMPONENT_DECLARE(VkImageViewArr);
ECS_COMPONENT_DECLARE(VkFramebuffer);
ECS_COMPONENT_DECLARE(VkSwapchainKHR);
ECS_COMPONENT_DECLARE(SurfaceFormat);

void registerSwapchain(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, VkSwapchainKHR);
    ECS_COMPONENT_DEFINE(ecs, VkImageViewArr);
    ECS_COMPONENT_DEFINE(ecs, VkFramebuffer);
    ECS_COMPONENT_DEFINE(ecs, SurfaceFormat);
    ECS_PREFAB_DEFINE(ecs, Swapchain, VkSwapchainKHR, VkImageViewArr, VkFramebuffer, SurfaceFormat);
}

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

static void
_setupImageViews(ecs_world_t* ecs, ecs_entity_t eSwapchain,
    VkDevice device, VkSwapchainKHR swapchain, int format)
{
    uint32_t count;
    VkImage* images = NULL;
    VkImageViewArr views = NULL;

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
        views[i] = _VkImageView(device, images[i], data);
        ecs_trace("Created VkImageView = %#p", images[i]);
    }
    ecs_set_ptr(ecs, eSwapchain, VkImageViewArr, views);
}

void setupSwapchain(ecs_world_t* ecs, ecs_entity_t eSystem,
    int requestedImages, bool vsync, uint32_t defaultWidth, uint32_t defaultHeight)
{
    assert(ecs_has_pair(ecs, eSystem, EcsIsA, VulkanSystem));
    ecs_entity_t e = ecs_get_target(ecs, eSystem, Swapchain, 0);

    VkSurfaceKHR surface = *ecs_get(ecs, eSystem, VkSurfaceKHR);
    ecs_entity_t eSelected = *ecs_get(ecs, eSystem, SelectedPhysicalDevice);
    VkPhysicalDevice physDev = *ecs_get(ecs, eSelected, VkPhysicalDevice);
    VkDevice device = *ecs_get(ecs, eSystem, VkDevice);
    ecs_trace("Creating Swapchain on device [%#p]", device);
    ecs_log_push();

    VkSurfaceCapabilitiesKHR capabilities = _getSurfaceCapabilitiesKHR(physDev, surface);
    int numImages = _calcNumImages(capabilities, requestedImages);
    SurfaceFormat format = _calcSurfaceFormat(physDev, surface);
    VkExtent2D extent = _calcSwapchainExtent(capabilities, defaultWidth, defaultHeight);

    VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
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
    if (vsync) {
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
    ecs_set_ptr(ecs, e, VkSwapchainKHR, &swapchain);
    ecs_set_ptr(ecs, e, SurfaceFormat, &format);

    _setupImageViews(ecs, e, device, swapchain, format.imageFormat);

    ecs_log_pop();
}

void cleanupSwapchain(ecs_world_t* ecs, ecs_entity_t eSystem)
{
    assert(ecs_has_pair(ecs, eSystem, EcsIsA, VulkanSystem));
    ecs_trace("Destroying Vulkan swapchain");
    VkDevice device = *ecs_get(ecs, eSystem, VkDevice);
    VkImageViewArr views = *ecs_get(ecs, eSystem, VkImageViewArr);
    VkSwapchainKHR swapchain = *ecs_get(ecs, eSystem, VkSwapchainKHR);
    for (int i = 0; i < arrlen(views); ++i) {
        vkDestroyImageView(device, views[i], NULL);
    }
    arrfree(views);
    assert(views == NULL);
    ecs_set_ptr(ecs, eSystem, VkImageViewArr, &views);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    swapchain = NULL;
    ecs_set_ptr(ecs, eSystem, VkSwapchainKHR, &swapchain);
}
