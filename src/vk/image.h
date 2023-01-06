#pragma once

#include <vulkan/vulkan.h>

typedef struct Device Device;

typedef struct ImageInfo {
    VkFormat format;
    int width;
    int height;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    VkSampleCountFlagBits samples;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkSharingMode sharingMode;
} ImageInfo;

typedef struct Image {
    VkImage handle;
    const Device* pDevice;
    ImageInfo info;
} Image;

Image newImage(const Device* pDevice, const ImageInfo* pInfo, const void* pData, VkDeviceSize size);

typedef struct ImageView {
    VkImageView handle;
    VkImage image;
    VkFence fence;
    VkSemaphore acquisitionSemaphore;
    VkSemaphore renderCompleteSemaphore;
} ImageView;