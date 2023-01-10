#include "framebuffer.h"

#include <flecs.h>
#include <stdlib.h>

#include "check.h"
#include "device.h"
#include "image.h"
#include "render_pass.h"

void createFramebuffer(const FramebufferCreateInfo* pCreateInfo, Framebuffer* pFramebuffer)
{
    VkFramebufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = pCreateInfo->pRenderPass->vkRenderPass,
        .attachmentCount = 1,
        .pAttachments = &pCreateInfo->pImageView->handle,
        .width = pCreateInfo->width,
        .height = pCreateInfo->height,
        .layers = 1,
    };
    VkFramebuffer fb;
    vkIfFailed(vkCreateFramebuffer(pCreateInfo->pRenderPass->pDevice->vkDevice, &ci, NULL, &fb))
    {
        ecs_abort(1, "Failed to create VkFramebuffer");
    }
    *pFramebuffer = (Framebuffer) {
        .vkFramebuffer = fb,
        .pDevice = pCreateInfo->pRenderPass->pDevice,
        .pRenderPass = pCreateInfo->pRenderPass,
        .pImageView = pCreateInfo->pImageView,
        .width = pCreateInfo->width,
        .height = pCreateInfo->height,
    };
}

void destroyFramebuffer(Framebuffer* pFramebuffer)
{
    vkDestroyFramebuffer(pFramebuffer->pDevice->vkDevice, pFramebuffer->vkFramebuffer, NULL);
    *pFramebuffer = (Framebuffer) {0};
}
