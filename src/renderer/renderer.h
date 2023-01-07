#pragma once

#include <vulkan/vulkan.h>

#include "clear_screen.h"

typedef struct RendererRecordInfo {
    VkCommandBuffer commandBuffer;
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;
    VkExtent2D extent;
} RendererRecordInfo;