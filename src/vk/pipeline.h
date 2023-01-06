#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct Device Device;
typedef struct RenderPass RenderPass;
typedef struct ShaderProgram ShaderProgram;
typedef struct VertexInputStateInfo VertexInputStateInfo;
typedef struct DescriptorSetLayout DescriptorSetLayout;

typedef struct PipelineCache {
    VkPipelineCache vkPipelineCache;
    const Device* pDevice;
} PipelineCache;

void createPipelineCache(const Device* pDevice, PipelineCache* pPipelineCache);

typedef struct Pipeline {
    VkPipeline vkPipeline;
    VkPipelineLayout vkPipelineLayout;
    const Device* pDevice;
} Pipeline;

typedef struct PipelineCreateInfo {
    const RenderPass* pRenderPass;
    const ShaderProgram* pShaderProgram;
    int colorAttachmentCount;
    int depthAttachmentCount;
    bool blend;
    uint32_t pushConstantSize;
    const VertexInputStateInfo* pVertexInputStateInfo;
    uint32_t descriptorSetLayoutCount;
    const DescriptorSetLayout* pDescriptorSetLayout;
} PipelineCreateInfo;

void createPipeline(const PipelineCreateInfo* pCreateInfo, Pipeline* pPipeline);