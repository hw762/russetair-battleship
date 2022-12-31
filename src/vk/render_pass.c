#include "render_pass.h"
#include "vk.h"

#include <flecs.h>
#include <stdlib.h>

#include "device.h"
#include "swapchain.h"

RenderPass
newRenderPassClearSwapchain(const RenderDevice* device, const Swapchain* swapchain)
{
    VkAttachmentDescription attachments[1] = {
        { .format = swapchain->format.imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
    };
    VkAttachmentReference colorReference[1] = {
        { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
    };
    VkSubpassDescription subpass[1] = {
        { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = colorReference }
    };
    VkSubpassDependency subpassDependencies[1] = {
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
        .pDependencies = subpassDependencies,
    };
    VkRenderPass renderPass;
    vkCheck(vkCreateRenderPass(device->handle, &ci, NULL, &renderPass))
    {
        ecs_abort(1, "Failed to create Render Pass");
    }
    return (RenderPass) {
        .handle = renderPass,
        .device = device,
    };
}