package gfx.gui


import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK13.vkCmdBeginRendering
import org.lwjgl.vulkan.VK13.vkCmdEndRendering


class NuklearRenderer(val device: Device, val pipelineCache: PipelineCache, val colorFormat: Int)
{

    private val nkState = NuklearState(device)
    private val pipeline = NuklearPipeline(device, pipelineCache, colorFormat)

    fun cleanup() {
        pipeline.cleanup()
        nkState.cleanup()
    }

    fun render(cmdBuf: VkCommandBuffer, outImageView: Long, renderArea: VkRect2D) {
        beginRecord(cmdBuf)
        beginRendering(cmdBuf, outImageView, renderArea)
        // TODO: bind pipeline and draw state
        endRendering(cmdBuf)
        endRecord(cmdBuf)
    }

    private fun beginRecord(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val beginInfo = VkCommandBufferBeginInfo.calloc(stack)
                .`sType$Default`()
                .flags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
            nkState.prepareRenderActivity()
            vkCheck(vkBeginCommandBuffer(cmdBuf, beginInfo), "Failed to begin command buffer")
        }
    }
    private fun beginRendering(cmdBuf: VkCommandBuffer, outImageView: Long, renderArea: VkRect2D) {
        MemoryStack.stackPush().use { stack ->
            val colorAttachmentInfo = VkRenderingAttachmentInfo.calloc(1, stack)
                .`sType$Default`()
                .imageView(outImageView)
                .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
                .clearValue {
                    it.color()
                        .float32(0, 0.1f)
                        .float32(1,0.2f)
                        .float32(2, 0.3f)
                        .float32(3, 1.0f)
                }
            val renderingInfo = VkRenderingInfo.calloc(stack)
                .`sType$Default`()
                .renderArea(renderArea)
                .layerCount(1)
                .pColorAttachments(colorAttachmentInfo)
            vkCmdBeginRendering(cmdBuf, renderingInfo)
        }
    }
    private fun endRendering(cmdBuf: VkCommandBuffer) {
        vkCmdEndRendering(cmdBuf)
    }
    private fun endRecord(cmdBuf: VkCommandBuffer) {
        vkEndCommandBuffer(cmdBuf)
        nkState.finalizeRenderActivity()
    }
}