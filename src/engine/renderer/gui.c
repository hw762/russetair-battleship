#include "gui.h"

#include <flecs.h>
#include <stdlib.h>

#include "engine/vk/check.h"
#include "vulkan/vulkan_core.h"

struct GuiRenderer_T {
    struct nk_context ctx;
    VkDevice device;
    VmaAllocator allocator;
    VkImage fontAtlasImage;
    VmaAllocation fontAtlasImageAlloc;
    VkImageView fontAtlasView;
};

void createGuiRenderer(const GuiRendererCreateInfo* pInfo, GuiRenderer* pRenderer)
{
    struct GuiRenderer_T* renderer = malloc(sizeof(*renderer));
    if (renderer == NULL) {
        ecs_abort(1, "Failed to malloc [GuiRenderer]");
    }
    // Initialize Nuklear context
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_font* font;
    nk_font_atlas_init_default(&atlas);
    font = nk_font_atlas_add_default(&atlas, 13, 0);
    int w, h;
    const void* image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

    // Create texture
    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = { .width = pInfo->width, .height = pInfo->height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = VK_FORMAT_R8G8B8A8_UINT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };
    VmaAllocationCreateInfo allocCI = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    VkImage vkImage;
    VmaAllocation vmaAlloc;
    vkCheck(vmaCreateImage(pInfo->allocator, &imageCI, &allocCI, &vkImage, &vmaAlloc, NULL))
    {
        ecs_abort(1, "Failed to allocate image for [GuiRenderer]");
    }
    // Staging
    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = w * h * 4,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo stagingAllocCreateInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufAlloc;
    VmaAllocationInfo stagingBufAllocInfo;
    vkCheck(vmaCreateBuffer(pInfo->allocator, &stagingInfo, &stagingAllocCreateInfo, &stagingBuffer, &stagingBufAlloc, &stagingBufAllocInfo))
    {
        ecs_abort(1, "Failed to allocate transfer buffer");
    }
    void* pTransfer = stagingBufAllocInfo.pMappedData;
    memcpy(pTransfer, image, w * h * 4);
    // Upload to device
    VkCommandBufferBeginInfo cmdBI
        = {
              .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
              .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
          };
    vkCheck(vkBeginCommandBuffer(pInfo->cmdBuffer, &cmdBI))
    {
        ecs_abort(1, "Failed to start command");
    }

    VkImageMemoryBarrier imgMemBarrier = {
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = vkImage,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    };

    vkCmdPipelineBarrier(pInfo->cmdBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, NULL, 0, NULL, 1, &imgMemBarrier);

    VkBufferImageCopy region = {
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .imageExtent = {
            .width = w,
            .height = h,
            .depth = 1,
        }
    };

    vkCmdCopyBufferToImage(pInfo->cmdBuffer, stagingBuffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgMemBarrier.image = vkImage;
    imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(pInfo->cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, NULL, 0, NULL, 1, &imgMemBarrier);

    vkEndCommandBuffer(pInfo->cmdBuffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &pInfo->cmdBuffer,
    };
    vkCheck(vkQueueSubmit(pInfo->queue, 1, &submitInfo, VK_NULL_HANDLE))
    {
        ecs_abort(1, "Failed to submit image transfer");
    }
    vkCheck(vkQueueWaitIdle(pInfo->queue))
    {
        ecs_abort(1, "Failed to execute transfer");
    }

    vmaDestroyBuffer(pInfo->allocator, stagingBuffer, stagingBufAlloc);

    // Create image view
    VkImageView view;
    VkImageViewCreateInfo viewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vkImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UINT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    vkCheck(vkCreateImageView(pInfo->device, &viewCI, NULL, &view))
    {
        ecs_abort(1, "Failed to create image view");
    }

    // Clean up

    nk_font_atlas_end(&atlas, nk_handle_id(0), NULL);
    nk_init_default(&ctx, &font->handle);
    *renderer = (struct GuiRenderer_T) {
        .ctx = ctx,
        .device = pInfo->device,
        .allocator = pInfo->allocator,
        .fontAtlasImage = vkImage,
        .fontAtlasView = view,
    };
    *pRenderer = renderer;
}

void destroyGuiRenderer(GuiRenderer r)
{
    vkDestroyImageView(r->device, r->fontAtlasView, NULL);
    vmaDestroyImage(r->allocator, r->fontAtlasImage, r->fontAtlasImageAlloc);
    free(r);
}