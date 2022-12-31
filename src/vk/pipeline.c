#include "pipeline.h"
#include "vk.h"

ECS_PREFAB_DECLARE(GraphicsPipeline);
ECS_COMPONENT_DECLARE(VkPipeline);
ECS_COMPONENT_DECLARE(VkRenderPass);
ECS_COMPONENT_DECLARE(VkDescriptorSetLayout);
ECS_COMPONENT_DECLARE(VertexShaderModule);
ECS_COMPONENT_DECLARE(FragmentShaderModule);
ECS_COMPONENT_DECLARE(VkCommandPool);

void registerPipeline(ecs_world_t* ecs)
{

    ECS_COMPONENT_DEFINE(ecs, VkPipeline);
    ECS_COMPONENT_DEFINE(ecs, VkRenderPass);
    ECS_COMPONENT_DEFINE(ecs, VkDescriptorSetLayout);
    ECS_COMPONENT_DEFINE(ecs, VertexShaderModule);
    ECS_COMPONENT_DEFINE(ecs, FragmentShaderModule);
    ECS_COMPONENT_DEFINE(ecs, VkCommandPool);
    ECS_PREFAB_DEFINE(ecs, GraphicsPipeline,
        VkPipeline, VkRenderPass, VkDescriptorSetLayout,
        VertexShaderModule, FragmentShaderModule, VkCommandPool);
}
