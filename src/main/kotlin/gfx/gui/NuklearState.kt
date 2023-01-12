package gfx.gui

import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.nuklear.*
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.util.vma.Vma
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*

class NuklearState(private val device: Device) :
    RenderActivity, UploadActivity {
    private val ctx = NkContext.create()
    private val defaultFont = NuklearFont(device, DEFAULT_FONT, 18.0f, FONT_BITMAP_W, FONT_BITMAP_H)
    private val cmds = NkBuffer.create()
    private val nullTexture = NkDrawNullTexture.create()

    private val vkSampler: Long

    private val vertices = VulkanBuffer(
        device, MAX_VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Vma.VMA_MEMORY_USAGE_AUTO, Vma.VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )
    private val elements = VulkanBuffer(
        device, MAX_ELEMENT_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Vma.VMA_MEMORY_USAGE_AUTO, Vma.VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )

    private val vkDescriptorPool: Long
    private val projMatDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT
    )
    private val textureDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_SHADER_STAGE_FRAGMENT_BIT
    )

    /// A mapping from VkImageView to VkDescriptorSet. Cleared every frame.
    private val boundTextures = HashMap<Long, Long>()

    init {
        nk_init(ctx, ALLOCATOR, defaultFont.nkFont)
        MemoryStack.stackPush().use { stack ->
            vkDescriptorPool = createDescriptorPool(stack, device)
            vkSampler = createTextureSampler(stack)
        }

    }

    private fun createTextureSampler(stack: MemoryStack): Long {
        val lp = stack.mallocLong(1)
        val samplerCI = VkSamplerCreateInfo.calloc(stack)
            .`sType$Default`()
            .magFilter(VK_FILTER_NEAREST)
            .minFilter(VK_FILTER_NEAREST)
            .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
            .addressModeU(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .addressModeV(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .addressModeW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
        vkCheck(
            vkCreateSampler(device.vkDevice, samplerCI, null, lp),
            "Failed to create sampler"
        )
        return lp[0]
    }

    fun cleanup() {
        vkDestroySampler(device.vkDevice, vkSampler, null)
        defaultFont.cleanup()
        vertices.cleanup()
        elements.cleanup()
        nullTexture.free()
        nk_buffer_free(cmds)
        projMatDescriptorSetLayout.cleanup()
        textureDescriptorSetLayout.cleanup()
        vkDestroyDescriptorPool(device.vkDevice, vkDescriptorPool, null)
    }

    /**
     * Convert and write Nuklear commands into vertex and index buffer.
     */
    private fun writeVerticesIndicesBuffer() {
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

    /**
     * Binds a texture to descriptor set and returns the VkDescriptorSet handle.
     * If already bound, return existing descriptor set.
     */
    private fun textureDescriptorSet(vkImageView: Long): Long {
        val existing = boundTextures[vkImageView]
        if (existing != null) {
            return existing
        }
        MemoryStack.stackPush().use { stack ->
            // Allocate descriptor set
            val pLayouts = stack.mallocLong(1)
            pLayouts.put(0, textureDescriptorSetLayout.vkDescriptorLayout)
            val descriptorSetAI = VkDescriptorSetAllocateInfo.calloc(stack)
                .`sType$Default`()
                .descriptorPool(vkDescriptorPool)
                .pSetLayouts(pLayouts)
            val lp = stack.mallocLong(1)
            vkCheck(vkAllocateDescriptorSets(device.vkDevice, descriptorSetAI, lp),
                "Failed to allocate descriptor set")
            val vkDescriptorSet = lp[0]
            boundTextures[vkImageView] = vkDescriptorSet
            // Write descriptor set
            val descriptorImageInfo = VkDescriptorImageInfo.calloc(1, stack)
                .sampler(vkSampler)
                .imageView(vkImageView)
                .imageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            val writeInfo = VkWriteDescriptorSet.calloc(1, stack)
                .`sType$Default`()
                .dstSet(vkDescriptorSet)
                .dstBinding(0)
                .descriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                .pImageInfo(descriptorImageInfo)
            vkUpdateDescriptorSets(device.vkDevice, writeInfo, null)

            return vkDescriptorSet
        }
    }

    private fun drawNuklearCommands(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val lp = stack.mallocLong(1)
            lp.put(0, textureDescriptorSetLayout.vkDescriptorLayout)
            var cmd = nk__draw_begin(ctx, cmds)
            var offset = 0
            while (cmd != null) {
                if (cmd.elem_count() == 0)
                    continue
                // Bind texture
                val texView = cmd.texture().ptr()
                val descriptorSet = textureDescriptorSet(texView)
                val pDS = stack.mallocLong(1)
                pDS.put(0, descriptorSet)
                vkCmdBindDescriptorSets(
                    cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, textureDescriptorSetLayout.vkDescriptorLayout,
                    0, pDS, null
                )
                // Draw TODO: display scaling
                val scissors = VkRect2D.malloc(1, stack)
                    .extent {it
                        .width(cmd!!.clip_rect().w().toInt())
                        .height(cmd!!.clip_rect().h().toInt())
                    }
                    .offset { it
                        .x(cmd!!.clip_rect().x().toInt())
                        .y(cmd!!.clip_rect().y().toInt())
                    }
                vkCmdSetScissor(cmdBuf, 0, scissors)
                vkCmdDrawIndexed(cmdBuf, cmd.elem_count(), 1, offset, 0, 0)
                // Advance and get next command
                offset += cmd.elem_count()
                cmd = nk__draw_next(cmd, cmds, ctx)
            }
        }

    }

    companion object {
        const val DEFAULT_FONT = "resources/fonts/FiraSans-Regular.otf"
        private const val FONT_BITMAP_W = 1024
        private const val FONT_BITMAP_H = 1024
        private const val MAX_VERTEX_BUFFER = 512 * 1024L
        private const val MAX_ELEMENT_BUFFER = 128 * 1024L
        private const val MAX_TEXTURES = 128

        private val VERTEX_LAYOUT: NkDrawVertexLayoutElement.Buffer = NkDrawVertexLayoutElement.create(4)
            .position(0).attribute(NK_VERTEX_POSITION).format(NK_FORMAT_FLOAT).offset(0)
            .position(1).attribute(NK_VERTEX_TEXCOORD).format(NK_FORMAT_FLOAT).offset(8)
            .position(2).attribute(NK_VERTEX_COLOR).format(NK_FORMAT_R8G8B8A8).offset(16)
            .position(3).attribute(NK_VERTEX_ATTRIBUTE_COUNT).format(NK_FORMAT_COUNT).offset(0)
            .flip()
        private val ALLOCATOR: NkAllocator = NkAllocator.create()
            .alloc { _: Long, _: Long, size: Long ->
                MemoryUtil.nmemAllocChecked(
                    size
                )
            }
            .mfree { _: Long, ptr: Long -> MemoryUtil.nmemFree(ptr) }

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

    override fun prepareRenderActivity() {
        writeVerticesIndicesBuffer()
    }

    override fun recordRenderActivity(cmdBuf: VkCommandBuffer) {
        drawNuklearCommands(cmdBuf)
    }

    override fun finalizeRenderActivity() {
        nk_clear(ctx)
        nk_buffer_clear(cmds)
        vkResetDescriptorPool(device.vkDevice, vkDescriptorPool, 0)
        boundTextures.clear()
    }

    override fun recordUpload(cmdBuf: VkCommandBuffer) {
        defaultFont.recordUpload(cmdBuf)
    }

    override fun finalizeAfterUpload() {
        defaultFont.finalizeAfterUpload()
    }
}