#pragma once

#include <vulkan/vulkan.h>

typedef struct RenderPass RenderPass;
typedef struct ImageView ImageView;
typedef struct Device Device;

typedef struct Framebuffer {
    VkFramebuffer vkFramebuffer;
    const Device* pDevice;
    const RenderPass* pRenderPass;
    const ImageView* pImageView;
    uint32_t width;
    uint32_t height;

} Framebuffer;

typedef struct FramebufferCreateInfo {
    const RenderPass* pRenderPass;
    const ImageView* pImageView;
    uint32_t width;
    uint32_t height;
} FramebufferCreateInfo;

void createFramebuffer(const FramebufferCreateInfo* pCreateInfo, Framebuffer* pFramebuffer);
void destroyFramebuffer(Framebuffer* pFramebuffer);