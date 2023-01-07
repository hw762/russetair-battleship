#pragma once

#include <nuklear.h>
#include <stdint.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

typedef struct GuiRendererCreateInfo {
    VkDevice device;
    VmaAllocator allocator;
    VkCommandBuffer cmdBuffer;
    VkQueue queue;
    uint32_t width;
    uint32_t height;
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