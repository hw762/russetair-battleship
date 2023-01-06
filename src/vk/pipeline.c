#include "pipeline.h"

#include <flecs.h>
#include <stdlib.h>

#include "check.h"
#include "device.h"

void createPipelineCache(const Device* pDevice, PipelineCache* pPipelineCache)
{
    ecs_trace("Creating pipeline cache");
    ecs_log_push();
    VkPipelineCacheCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    };
    VkPipelineCache c;
    vkCheck(vkCreatePipelineCache(pDevice->handle, &ci, NULL, &c))
    {
        ecs_abort(1, "Failed to create VkPipelineCache");
    }
    *pPipelineCache = (PipelineCache) {
        .vkPipelineCache = c,
        .pDevice = pDevice,
    };
    ecs_log_pop();
}

void destroyPipelineCache(PipelineCache* pPipelineCache)
{
    vkDestroyPipelineCache(pPipelineCache->pDevice->handle, pPipelineCache->vkPipelineCache, NULL);
    *pPipelineCache = (PipelineCache) {};
}

void createPipeline(const PipelineCreateInfo* pCreateInfo, Pipeline* pPipeline)
{
    ecs_trace("Creating pipeline");
    ecs_log_push();

    /// TODO

    ecs_log_pop();
}
