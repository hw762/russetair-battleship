#pragma once

#include "nuklear.h"
#include "vulkan/vulkan.h"

typedef struct GuiRendererCreateInfo {
} GuiRendererCreateInfo;

typedef struct GuiRenderer_T* GuiRenderer;

void createGuiRenderer(const GuiRendererCreateInfo* pInfo, GuiRenderer* pRenderer);
void destroyGuiRenderer(GuiRenderer renderer);

typedef struct GuiRendererRecordInfo {
    VkCommandBuffer commandBuffer;
    VkFramebuffer framebuffer;
    VkImageView view;
    uint32_t width;
    uint32_t height;
} GuiRendererRecordInfo;