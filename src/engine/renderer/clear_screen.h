#pragma once

#include "engine/renderer/renderer.h"
#include <vulkan/vulkan.h>

typedef struct RendererRecordInfo RendererRecordInfo;

typedef struct ClearScreenRendererCreateInfo {
    const VkClearColorValue clearColor;
} ClearScreenRendererCreateInfo;

typedef struct ClearScreenRenderer_T* ClearScreenRenderer;

void createClearScreenRenderer(const ClearScreenRendererCreateInfo* pCreateInfo, ClearScreenRenderer* pRenderer);

void destroyClearScreenRenderer(ClearScreenRenderer renderer);

void clearScreenRendererRecord(
    ClearScreenRenderer renderer,
    const RendererRecordInfo* pInfo);