#pragma once

#include "engine/renderer/renderer.h"
#include <vulkan/vulkan.h>

typedef struct ClearScreenRendererCreateInfo {
    const VkClearColorValue clearColor;
} ClearScreenRendererCreateInfo;

typedef struct ClearScreenRenderer_T* ClearScreenRenderer;

void createClearScreenRenderer(const ClearScreenRendererCreateInfo* pCreateInfo, ClearScreenRenderer* pRenderer);

void destroyClearScreenRenderer(ClearScreenRenderer renderer);

typedef struct ClearScreenRendererRecordInfo {
    VkCommandBuffer commandBuffer;
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;
    VkExtent2D extent;
} ClearScreenRendererRecordInfo;

void clearScreenRendererRecord(
    ClearScreenRenderer renderer,
    const ClearScreenRendererRecordInfo* pInfo);