#pragma once

#include <vulkan/vulkan.h>

typedef struct ClearScreenDynRendererCreateInfo {
    const VkClearColorValue clearColor;
} ClearScreenDynRendererCreateInfo;

typedef struct ClearScreenDynRenderer_T* ClearScreenDynRenderer;

void createClearScreenDynRenderer(const ClearScreenDynRendererCreateInfo* pCreateInfo, ClearScreenDynRenderer* pRenderer);

void destroyClearScreenDynRenderer(ClearScreenDynRenderer renderer);

typedef struct ClearScreenDynRendererRecordInfo {
    VkCommandBuffer commandBuffer;
    VkImageView view;
    uint32_t width;
    uint32_t height;
} ClearScreenDynRendererRecordInfo;

void clearScreenDynRendererRecord(
    ClearScreenDynRenderer renderer,
    const ClearScreenDynRendererRecordInfo* pInfo);