#include "gui.h"

#include <flecs.h>
#include <nuklear.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "engine/vk/check.h"
#include "vulkan/vulkan_core.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

struct NkVertex {
    float position[2];
    float uv[2];
    nk_byte rgba[4];
};

struct GuiRenderer_T {
    struct nk_context ctx;
    VkDevice device;
    VmaAllocator allocator;
    // Fonts
    struct nk_font_atlas atlas;
    VkImage fontAtlasImage;
    VmaAllocation fontAtlasImageAlloc;
    VkImageView fontAtlasView;
    struct nk_draw_null_texture tex_null;
    //
    struct nk_buffer cmds;
    VkBuffer verticesBuffer;
    VmaAllocation verticesBufferAlloc;
    VkBuffer elementsBuffer;
    VmaAllocation elementsBufferAlloc;
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
_transferAtlas(VkCommandBuffer cmdBuf, VkQueue queue, VkBuffer src,
    VkImage dst, uint32_t width, uint32_t height)
{
    VkCommandBufferBeginInfo cmdBI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkCheck(vkBeginCommandBuffer(cmdBuf, &cmdBI), "Failed to start command");

    VkImageMemoryBarrier imgMemBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
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
        .image = dst,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    };

    vkCmdPipelineBarrier(cmdBuf,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, NULL, 0, NULL, 1, &imgMemBarrier);

    VkBufferImageCopy region = {
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

    // First, copy to image
    vkCmdCopyBufferToImage(cmdBuf, src, dst,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgMemBarrier.image = dst;
    imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // Then, convert layout
    vkCmdPipelineBarrier(cmdBuf,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &imgMemBarrier);

    vkEndCommandBuffer(cmdBuf);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf,
    };
    vkCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE),
        "Failed to submit image transfer");
    vkCheck(vkQueueWaitIdle(queue), "Failed to execute transfer");
}

static void _initCtx(const GuiRendererCreateInfo* pInfo, GuiRenderer renderer)
{
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
    _transferAtlas(pInfo->cmdBuffer, pInfo->queue, stagingBuffer, vkImage, w,
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
    struct nk_draw_null_texture tex_null;
    nk_font_atlas_end(&atlas, nk_handle_ptr(renderer->fontAtlasView), &tex_null);
    nk_font_atlas_cleanup(&atlas);
    nk_init_default(&ctx, &font->handle);
}

static void _initBuffers(const GuiRendererCreateInfo* pInfo, GuiRenderer renderer)
{
    nk_buffer_init_default(&renderer->cmds);
    VkBufferCreateInfo vbufCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = MAX_VERTEX_BUFFER,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    };
    VmaAllocationCreateInfo aci = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    VmaAllocationInfo ai;
    // Allocate vertex buffer
    vkCheck(vmaCreateBuffer(pInfo->allocator, &vbufCI, &aci,
                &renderer->verticesBuffer, &renderer->verticesBufferAlloc, &ai),
        "Failed to create vertex buffer");
    // Allocate element buffer
    VkBufferCreateInfo ebufCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = MAX_ELEMENT_BUFFER,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    };
    vkCheck(vmaCreateBuffer(pInfo->allocator, &ebufCI, &aci,
                &renderer->elementsBuffer, &renderer->elementsBufferAlloc, &ai),
        "Failed to create element buffer");
}

void createGuiRenderer(const GuiRendererCreateInfo* pInfo, GuiRenderer* pRenderer)
{
    ecs_trace("Creating [GuiRenderer]");
    ecs_log_push();
    struct GuiRenderer_T* renderer = malloc(sizeof(*renderer));
    if (renderer == NULL) {
        ecs_abort(1, "Failed to malloc [GuiRenderer]");
    }
    *renderer = (struct GuiRenderer_T) {
        .device = pInfo->device,
        .allocator = pInfo->allocator,
    };

    _initCtx(pInfo, renderer);
    _initBuffers(pInfo, renderer);
    // Other structures

    // Output
    *pRenderer = renderer;
    ecs_log_pop();
}

void destroyGuiRenderer(GuiRenderer r)
{
    ecs_trace("Destroying [GuiRenderer]");
    nk_font_atlas_clear(&r->atlas);
    vkDestroyImageView(r->device, r->fontAtlasView, NULL);
    vmaDestroyImage(r->allocator, r->fontAtlasImage, r->fontAtlasImageAlloc);
    free(r);
}

static void _convertDrawCommands(GuiRenderer r)
{
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NkVertex, position) },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NkVertex, uv) },
        { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct NkVertex, rgba) },
        { NK_VERTEX_LAYOUT_END }
    };

    struct nk_convert_config config = {
        .vertex_layout = vertex_layout,
        .vertex_size = sizeof(struct NkVertex),
        .vertex_alignment = NK_ALIGNOF(struct NkVertex),
        .tex_null = r->tex_null,
        .circle_segment_count = 22,
        .curve_segment_count = 22,
        .arc_segment_count = 22,
        .global_alpha = 1.0f,
        .shape_AA = NK_ANTI_ALIASING_OFF,
        .line_AA = NK_ANTI_ALIASING_OFF,
    };
    struct nk_buffer vbuf, ebuf;
    void *vertices, *elements;
    vmaMapMemory(r->allocator, r->verticesBufferAlloc, &vertices);
    vmaMapMemory(r->allocator, r->elementsBufferAlloc, &elements);

    nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_BUFFER);
    nk_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_BUFFER);
    nk_convert(&r->ctx, &r->cmds, &vbuf, &ebuf, &config);

    vmaUnmapMemory(r->allocator, r->verticesBufferAlloc);
    vmaUnmapMemory(r->allocator, r->elementsBufferAlloc);
}

void guiRendererRecord(GuiRenderer renderer, const GuiRendererRecordInfo* pInfo)
{
    _convertDrawCommands(renderer);
}