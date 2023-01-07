#pragma once

#include <vulkan/vulkan.h>

typedef struct ClearScreenRendererCreateInfo {
    VkDevice device;
    uint32_t width;
    uint32_t height;
    const VkClearColorValue clearColor;
    VkFormat format;
} ClearScreenRendererCreateInfo;

typedef struct ClearScreenRenderer_T* ClearScreenRenderer;

void createClearScreenRenderer(const ClearScreenRendererCreateInfo* pCreateInfo, ClearScreenRenderer* pRenderer);

void destroyClearScreenRenderer(ClearScreenRenderer renderer);

typedef struct ClearScreenRendererRecordInfo {
    VkCommandBuffer commandBuffer;
    VkImageView view;
    uint32_t width;
    uint32_t height;
} ClearScreenRendererRecordInfo;

void clearScreenRendererRecord(
    ClearScreenRenderer renderer,
    const ClearScreenRendererRecordInfo* pInfo);