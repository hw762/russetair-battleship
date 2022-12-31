#include "pipeline.h"
#include "swapchain.h"
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

void setupSwapchainOutRenderPass(ecs_world_t* ecs, ecs_entity_t ePipeline,
    VkDevice device, SurfaceFormat format)
{
    assert(ecs_has_pair(ecs, ePipeline, EcsIsA, GraphicsPipeline));
    const int n_subpasses = 1;
    VkAttachmentDescription attachments[n_subpasses] = {
        { .format = format.imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
    };
    VkAttachmentReference colorReference[n_subpasses] = {
        { .attachment = 0, .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL }
    };
    VkSubpassDescription subpass[n_subpasses] = {
        { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = n_subpasses,
            .pColorAttachments = colorReference }
    };
    VkSubpassDependency subpassDependencies[n_subpasses] = {
        { .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT }
    };
    VkRenderPassCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = attachments,
        .pSubpasses = subpass,
        .pDependencies = subpassDependencies
    };
    VkRenderPass vkRenderPass;
    vkCheck(vkCreateRenderPass(device, &ci, NULL, &vkRenderPass))
    {
        ecs_abort(1, "Failed to create render pass");
    }
    ecs_set_ptr(ecs, ePipeline, VkRenderPass, &vkRenderPass);
}