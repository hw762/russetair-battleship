#include "render_pass.h"

#include <flecs.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "check.h"
#include "device.h"

void createDefaultRenderPass(const RenderPassCreateInfo* pCreateInfo, RenderPass* pRenderPass)
{
    VkAttachmentDescription colorAttachment = {
        .format = pCreateInfo->format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorReference,
    };
    VkRenderPassCreateInfo renderPassCI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    VkRenderPass vkRenderPass;
    vkCheck(vkCreateRenderPass(pCreateInfo->pDevice->handle, &renderPassCI, NULL, &vkRenderPass))* pRenderPass = (RenderPass) {
        .vkRenderPass = vkRenderPass,
        .pDevice = pCreateInfo->pDevice,
    };
}

void destroyRenderPass(RenderPass* renderPass)
{
    vkDestroyRenderPass(renderPass->pDevice->handle, renderPass->vkRenderPass, NULL);
    renderPass->vkRenderPass = VK_NULL_HANDLE;
}