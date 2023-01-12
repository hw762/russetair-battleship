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

    private val nkState = NuklearState(device)
    private val pipeline = NuklearPipeline(device, pipelineCache, colorFormat)

    init {
        MemoryStack.stackPush().use { stack ->

        }
    }

    fun cleanup() {
        pipeline.cleanup()
        nkState.cleanup()
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




    }
}