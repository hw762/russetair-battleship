#include "gui.h"

#include <flecs.h>
#include <nuklear.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "engine/vk/check.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define MAX_TEXTURES 128

struct NkVertex {
    float position[2];
    float uv[2];
    nk_byte rgba[4];
};

struct GuiRenderer_T {
    VkDevice device;
    VmaAllocator allocator;
    VkPipelineCache pipelineCache;
    struct nk_context ctx;
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
    //
    VkRenderPass renderPass;
    //
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
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

static void _initRenderPass(const GuiRendererCreateInfo* pInfo, GuiRenderer r)
{
    VkAttachmentDescription attachments[1] = {
        { .format = pInfo->format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
    };
    VkAttachmentReference colorReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDependency subpassDependencies[1] = {
        (VkSubpassDependency) {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

        }
    };
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorReference,
    };
    VkRenderPassCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = subpassDependencies,
    };
    vkCheck(vkCreateRenderPass(r->device, &ci, NULL, &r->renderPass),
        "Failed to create render pass");
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

static void _initDescriptorSets(const GuiRendererCreateInfo* pInfo, GuiRenderer r)
{
    VkDescriptorSetLayoutBinding bindings[] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
    };
    VkDescriptorSetLayoutCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = bindings,
    };
    vkCheck(vkCreateDescriptorSetLayout(r->device, &ci, NULL, &r->descriptorSetLayout),
        "Failed to create descriptor set layout");
    VkDescriptorPoolCreateInfo dpCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes = (VkDescriptorPoolSize[]) {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = MAX_TEXTURES,
            } },
        .maxSets = 1 + MAX_TEXTURES,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    };
    vkCheck(vkCreateDescriptorPool(pInfo->device, &dpCI, NULL, &r->descriptorPool), "Failed to create descriptor pool");
}

static void _initPipeline(const GuiRendererCreateInfo* pInfo, GuiRenderer r)
{
    VkPipelineLayoutCreateInfo plCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &r->descriptorSetLayout,
    };
    vkCheck(vkCreatePipelineLayout(r->device, &plCI, NULL, &r->pipelineLayout),
        "Failed to create pipeline layout");

    VkPipelineInputAssemblyStateCreateInfo piasCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    VkPipelineRasterizationStateCreateInfo prsCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };
    VkPipelineColorBlendAttachmentState pcbas = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo pcbsCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &pcbas,
    };

    VkPipelineViewportStateCreateInfo pvsCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineMultisampleStateCreateInfo pmssCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo pdsCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynStates,
    };
    /// TODO: create shaders
    VkPipelineShaderStageCreateInfo shaderStages[2];
    VkVertexInputBindingDescription vertexInputInfo[1] = {
        { 0, sizeof(struct NkVertex), VK_VERTEX_INPUT_RATE_VERTEX }
    };
    VkVertexInputAttributeDescription vertexAttrDesc[3] = {
        { 0, 0, VK_FORMAT_R32G32_SFLOAT, NK_OFFSETOF(struct NkVertex, position) },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, NK_OFFSETOF(struct NkVertex, uv) },
        { 2, 0, VK_FORMAT_R8G8B8A8_UINT, NK_OFFSETOF(struct NkVertex, rgba) },
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = vertexInputInfo,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = vertexAttrDesc,
    };

    // Create Pipeline
    VkGraphicsPipelineCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &piasCI,
        .pViewportState = &pvsCI,
        .pRasterizationState = &prsCI,
        .pMultisampleState = &pmssCI,
        .pColorBlendState = &pcbsCI,
        .pDynamicState = &pdsCI,
        .layout = r->pipelineLayout,
        .renderPass = r->renderPass,
        .basePipelineIndex = -1,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    vkCheck(vkCreateGraphicsPipelines(r->device, pInfo->pipelineCache, 1, &ci, NULL, &r->pipeline),
        "Failed to create pipeline");
    vkDestroyShaderModule(r->device, shaderStages[0].module, NULL);
    vkDestroyShaderModule(r->device, shaderStages[1].module, NULL);
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
        .pipelineCache = pInfo->pipelineCache,
    };

    _initCtx(pInfo, renderer);
    _initBuffers(pInfo, renderer);
    _initDescriptorSets(pInfo, renderer);
    _initRenderPass(pInfo, renderer);
    _initPipeline(pInfo, renderer);

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

static void _bindTexture(GuiRenderer r, const GuiRendererRecordInfo* pInfo, VkImageView tex)
{
    VkDescriptorSet ds;
    VkDescriptorSetAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = r->descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &r->descriptorSetLayout,
    };
    vkCheck(vkAllocateDescriptorSets(r->device, &ai, &ds),
        "Failed to allocate descriptor set");
    vkCmdBindDescriptorSets(
        pInfo->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipelineLayout,
        3, 1, &ds, 0, VK_NULL_HANDLE);
}

static void _drawCommands(GuiRenderer r, const GuiRendererRecordInfo* pInfo)
{
    VkImageView currentTex = VK_NULL_HANDLE;
    uint32_t indexOffset = 0;
    const struct nk_draw_command* cmd;
    nk_draw_foreach(cmd, &r->ctx, &r->cmds)
    {
        if (!cmd->elem_count || !cmd->texture.ptr)
            continue;
        if (cmd->texture.ptr != currentTex)
            _bindTexture(r, pInfo, cmd->texture.ptr);
        VkRect2D scissor = {
            .extent = {
                .width = (uint32_t)cmd->clip_rect.w,
                .height = (uint32_t)cmd->clip_rect.h,
            },
            .offset = {
                .x = (uint32_t)NK_MAX(cmd->clip_rect.x, 0.f),
                .y = (uint32_t)NK_MAX(cmd->clip_rect.y, 0.f),
            }
        };
        vkCmdSetScissor(pInfo->commandBuffer, 0, 1, &scissor);
        vkCmdDrawIndexed(pInfo->commandBuffer, cmd->elem_count, 1, indexOffset, 0, 0);
        indexOffset += cmd->elem_count;
    }
}

void guiRendererRecord(GuiRenderer renderer, const GuiRendererRecordInfo* pInfo)
{
    _convertDrawCommands(renderer);
    _drawCommands(renderer, pInfo);
    vkCheck(vkResetDescriptorPool(renderer->device, renderer->descriptorPool, 0),
        "Failed to reset descriptor pool");
    nk_clear(&renderer->ctx);
}
