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

static void
_createAtlasImage(VmaAllocator allocator, uint32_t width, uint32_t height,
    VkImage* pVkImage, VmaAllocation* pVmaAlloc)
{
    assert(width != 0 && height != 0);
    VkImageCreateInfo imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = { .width = width, .height = height, .depth = 1 },
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
    vkCheck(vmaCreateImage(allocator, &imageCI, &allocCI, pVkImage,
                pVmaAlloc, NULL),
        "Failed to allocate image for [GuiRenderer]");
}

static void
_createTransferBuffer(VmaAllocator allocator, uint32_t pixels,
    VkBuffer* pBuffer, VmaAllocation* pAlloc,
    VmaAllocationInfo* pAllocInfo)
{
    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = pixels * 4,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo stagingAllocCreateInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };

    vkCheck(vmaCreateBuffer(allocator, &stagingInfo, &stagingAllocCreateInfo,
                pBuffer, pAlloc, pAllocInfo),
        "Failed to allocate transfer buffer");
}

static void
_transferToAtlas(VkCommandBuffer cmdBuf, VkQueue queue, VkBuffer src,
    VkImage dest, uint32_t width, uint32_t height)
{
    VkCommandBufferBeginInfo cmdBI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkCheck(vkBeginCommandBuffer(cmdBuf, &cmdBI), "Failed to start command");

    VkImageMemoryBarrier2 imgMemBarrier = {
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
        .image = dest,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
    };

    VkDependencyInfo dInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imgMemBarrier,
    };

    vkCmdPipelineBarrier2(cmdBuf, &dInfo);

    VkBufferImageCopy2 region = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .imageExtent = {
            .width = width,
            .height = height,
            .depth = 1,
        }
    };
    VkCopyBufferToImageInfo2 copyInfo = {
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .srcBuffer = src,
        .dstImage = dest,
        .regionCount = 1,
        .pRegions = &region,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    };

    // First, copy to image
    vkCmdCopyBufferToImage2(cmdBuf, &copyInfo);

    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    imgMemBarrier.image = dest;
    imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // Then, convert layout
    vkCmdPipelineBarrier2(cmdBuf, &dInfo);

    vkEndCommandBuffer(cmdBuf);
    VkSubmitInfo2 submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = (VkCommandBufferSubmitInfo[]) {
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = cmdBuf,
            },
        }
    };
    vkCheck(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE),
        "Failed to submit image transfer");
    vkCheck(vkQueueWaitIdle(queue), "Failed to execute transfer");
}

void createGuiRenderer(const GuiRendererCreateInfo* pInfo, GuiRenderer* pRenderer)
{
    ecs_trace("Creating [GuiRenderer]");
    ecs_log_push();
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
    const void* image
        = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    assert(w != 0 && h != 0);

    // Create atlas image
    VkImage vkImage;
    VmaAllocation vmaAlloc;
    _createAtlasImage(pInfo->allocator, w, h, &vkImage,
        &vmaAlloc);
    // Create staging buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufAlloc;
    VmaAllocationInfo stagingBufAllocInfo;
    _createTransferBuffer(pInfo->allocator, w * h, &stagingBuffer,
        &stagingBufAlloc, &stagingBufAllocInfo);
    // Copy data to transfer buffer
    void* pTransfer = stagingBufAllocInfo.pMappedData;
    memcpy(pTransfer, image, w * h * 4);
    // Upload to device
    _transferToAtlas(pInfo->cmdBuffer, pInfo->queue, stagingBuffer, vkImage, w,
        h);
    vmaDestroyBuffer(pInfo->allocator, stagingBuffer, stagingBufAlloc);

    // Create image view
    VkImageView view;
    VkImageViewCreateInfo viewCI
        = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
              .image = vkImage,
              .viewType = VK_IMAGE_VIEW_TYPE_2D,
              .format = VK_FORMAT_R8G8B8A8_UINT,
              .subresourceRange = {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              } };
    vkCheck(vkCreateImageView(pInfo->device, &viewCI, NULL, &view),
        "Failed to create image view");

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
    ecs_log_pop();
}

void destroyGuiRenderer(GuiRenderer r)
{
    ecs_trace("Destroying [GuiRenderer]");
    vkDestroyImageView(r->device, r->fontAtlasView, NULL);
    vmaDestroyImage(r->allocator, r->fontAtlasImage, r->fontAtlasImageAlloc);
    free(r);
}