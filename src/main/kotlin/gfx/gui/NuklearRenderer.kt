package gfx.gui


import gfx.vk.*
import org.lwjgl.nuklear.NkAllocator
import org.lwjgl.nuklear.NkBuffer
import org.lwjgl.nuklear.NkContext
import org.lwjgl.nuklear.NkDrawNullTexture
import org.lwjgl.nuklear.NkDrawVertexLayoutElement
import org.lwjgl.nuklear.NkUserFont
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.system.MemoryUtil.nmemAllocChecked
import org.lwjgl.system.MemoryUtil.nmemFree
import org.lwjgl.util.shaderc.Shaderc
import org.lwjgl.util.vma.*
import org.lwjgl.util.vma.Vma.*
import org.lwjgl.vulkan.VK10.*


class NuklearRenderer(val device: Device, val pipelineCache: PipelineCache) {
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

    private val renderPass: NuklearRenderPass
    private val vertexInfo: NuklearVertexInputStateInfo
    private val descriptorSetLayout: NuklearDescriptorSetLayout
    private val pipeline: Pipeline

    init {

        renderPass = NuklearRenderPass()
        vertexInfo = NuklearVertexInputStateInfo()
        descriptorSetLayout = NuklearDescriptorSetLayout()
        val pipelineCI = Pipeline.PipelineCreationInfo(
            renderPass.vkRenderPass, shaderProgram,
            1, false, false,
            0, vertexInfo, arrayOf(descriptorSetLayout),
        )
        pipeline = Pipeline(pipelineCache, pipelineCI)
    }

    fun cleanup() {
        vertices.cleanup()
        elements.cleanup()
        nk_buffer_free(cmds)
    }

    companion object {
        private const val MAX_VERTEX_BUFFER = 512 * 1024L
        private const val MAX_ELEMENT_BUFFER = 128 * 1024L

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

    }
}