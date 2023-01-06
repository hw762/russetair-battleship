#pragma once

#include <vulkan/vulkan.h>

typedef struct Device Device;

typedef struct RenderPass {
    VkRenderPass vkRenderPass;
    const Device* pDevice;
} RenderPass;

typedef struct RenderPassCreateInfo {
    const Device* pDevice;
    VkFormat format;
} RenderPassCreateInfo;

void createDefaultRenderPass(const RenderPassCreateInfo* pCreateInfo, RenderPass* pRenderPass);
void destroyRenderPass(RenderPass* renderPass);