package gfx.gui


import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.nuklear.*
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.system.MemoryUtil.nmemAllocChecked
import org.lwjgl.system.MemoryUtil.nmemFree
import org.lwjgl.util.vma.*
import org.lwjgl.util.vma.Vma.*
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.*


class NuklearRenderer(val device: Device, val pipelineCache: PipelineCache, val colorFormat: Int) {
    private val ctx = NkContext.create()
    private val defaultFont = NkUserFont.create()

    private val cmds = NkBuffer.create()
    private val nullTexture = NkDrawNullTexture.create()

    private val vertices = VulkanBuffer(
        device, MAX_VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )
    private val elements = VulkanBuffer(
        device, MAX_ELEMENT_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )
    private val vkDescriptorPool: Long
    private val projMatDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT
    )
    private val textureDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_SHADER_STAGE_FRAGMENT_BIT
    )
    private val vkSampler: Long

    private val pipeline = NuklearPipeline(device, pipelineCache, colorFormat)

    init {
        MemoryStack.stackPush().use { stack ->
            nk_init(ctx, ALLOCATOR, null)

            vkDescriptorPool = createDescriptorPool(stack, device)

            val samplerCI = VkSamplerCreateInfo.calloc(stack)
                .`sType$Default`()
                .magFilter(VK_FILTER_NEAREST)
                .minFilter(VK_FILTER_NEAREST)
                .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                .addressModeU(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                .addressModeV(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                .addressModeW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            vkCheck(vkCreateSampler(device.vkDevice, samplerCI, null, lp),
                "Failed to create sampler")
            vkSampler = lp[0]
        }
    }

    fun cleanup() {
        vertices.cleanup()
        elements.cleanup()
        nullTexture.free()
        defaultFont.free()
        nk_buffer_free(cmds)
        vkDestroyDescriptorPool(device.vkDevice, vkDescriptorPool, null)
        vkDestroySampler(device.vkDevice, vkSampler, null)
        pipeline.cleanup()
    }

    fun render(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val beginInfo = VkCommandBufferBeginInfo.calloc(stack)
                .`sType$Default`()
                .flags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
            vkCheck(vkBeginCommandBuffer(cmdBuf, beginInfo), "Failed to begin command buffer")
            convertCmds()
            uploadTextures(cmdBuf)
            nk_clear(ctx)
            nk_buffer_clear(cmds)
            vkEndCommandBuffer(cmdBuf)
            vkCheck(vkResetDescriptorPool(device.vkDevice, vkDescriptorPool, 0),
                "Failed to reset descriptor pool")
        }
    }

    /**
     * Convert and write Nuklear commands into vertex and index buffer.
     */
    private fun convertCmds() {
        val vertBuf = MemoryUtil.memByteBuffer(vertices.map(), vertices.requestedSize.toInt())
        val elemBuf = MemoryUtil.memByteBuffer(elements.map(), elements.requestedSize.toInt())

        MemoryStack.stackPush().use { stack ->
            val config = NkConvertConfig.calloc(stack)
                .global_alpha(1.0f)
                .line_AA(NK_ANTI_ALIASING_ON)
                .shape_AA(NK_ANTI_ALIASING_ON)
                .circle_segment_count(22)
                .arc_segment_count(22)
                .curve_segment_count(22)
                .tex_null(nullTexture)
                .vertex_layout(VERTEX_LAYOUT)
                .vertex_size(20)
                .vertex_alignment(4)
            val vbuf = NkBuffer.malloc(stack)
            val ebuf = NkBuffer.malloc(stack)
            nk_buffer_init_fixed(vbuf, vertBuf)
            nk_buffer_init_fixed(ebuf, elemBuf)
            nk_convert(ctx, cmds, vbuf, ebuf, config)
        }

        vertices.unmap()
        elements.unmap()
    }

    fun uploadTextures(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val lp = stack.mallocLong(1)
            lp.put(0, textureDescriptorSetLayout.vkDescriptorLayout)
            var cmd = nk__draw_begin(ctx, cmds)
            while (cmd != null) {
                if (cmd.elem_count() == 0)
                    continue
                /// TODO: bind texture
                val tex = cmd.texture().ptr()
                val descriptorSetCIs = VkDescriptorSetAllocateInfo.calloc(stack)
                    .`sType$Default`()
                    .descriptorPool(vkDescriptorPool)
                    .pSetLayouts(lp)
                val descriptorSets = stack.mallocLong(1)
                vkCheck(vkAllocateDescriptorSets(device.vkDevice, descriptorSetCIs, descriptorSets),
                    "Failed to allocate descriptor set")
                val descriptorImageInfo = VkDescriptorImageInfo.calloc(1, stack)
                    .sampler(vkSampler)
                    .imageView(tex)
                    .imageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                val writeInfo = VkWriteDescriptorSet.calloc(1, stack)
                    .`sType$Default`()
                    .dstSet(descriptorSets[0])
                    .dstBinding(0)
                    .descriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                    .pImageInfo(descriptorImageInfo)
                vkUpdateDescriptorSets(device.vkDevice, writeInfo, null)
                vkCmdBindDescriptorSets(
                    cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, textureDescriptorSetLayout.vkDescriptorLayout,
                    0, descriptorSets, null
                )
                //
                cmd = nk__draw_next(cmd, cmds, ctx)
            }
        }

    }

    companion object {
        private const val MAX_VERTEX_BUFFER = 512 * 1024L
        private const val MAX_ELEMENT_BUFFER = 128 * 1024L
        private const val MAX_TEXTURES = 128

        private val ALLOCATOR: NkAllocator = NkAllocator.create()
            .alloc { handle: Long, old: Long, size: Long ->
                nmemAllocChecked(
                    size
                )
            }
            .mfree { handle: Long, ptr: Long -> nmemFree(ptr) }
        private val VERTEX_LAYOUT: NkDrawVertexLayoutElement.Buffer = NkDrawVertexLayoutElement.create(4)
            .position(0).attribute(NK_VERTEX_POSITION).format(NK_FORMAT_FLOAT).offset(0)
            .position(1).attribute(NK_VERTEX_TEXCOORD).format(NK_FORMAT_FLOAT).offset(8)
            .position(2).attribute(NK_VERTEX_COLOR).format(NK_FORMAT_R8G8B8A8).offset(16)
            .position(3).attribute(NK_VERTEX_ATTRIBUTE_COUNT).format(NK_FORMAT_COUNT).offset(0)
            .flip();

        fun createDescriptorPool(stack: MemoryStack, device: Device): Long {
            val poolSizes = VkDescriptorPoolSize.calloc(2, stack)
            poolSizes[0]
                .type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                .descriptorCount(1)
            poolSizes[1]
                .type(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                .descriptorCount(MAX_TEXTURES)

            val descriptorPoolCI = VkDescriptorPoolCreateInfo.calloc(stack)
                .`sType$Default`()
                .maxSets(MAX_TEXTURES + 1)
                .pPoolSizes(poolSizes)
            val lp = stack.mallocLong(1)
            vkCheck(
                vkCreateDescriptorPool(device.vkDevice, descriptorPoolCI, null, lp),
                "Failed to create descriptor pool"
            )
            return lp[0]
        }
    }
}