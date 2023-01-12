package gfx.gui


import gfx.Renderer
import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK13.vkCmdBeginRendering
import org.lwjgl.vulkan.VK13.vkCmdEndRendering


class NuklearRenderer(val device: Device, val pipelineCache: PipelineCache, val colorFormat: Int)
    : Renderer {

    private val nkState = NuklearState(device)
    private val pipeline = NuklearPipeline(device, pipelineCache, colorFormat)

    fun cleanup() {
        pipeline.cleanup()
        nkState.cleanup()
    }

    override fun render(cmdBuf: VkCommandBuffer, outImage: Image, outImageView: ImageView, outLayout: Int, renderArea: VkRect2D) {
        beginRender(cmdBuf)
        transitionViewLayoutToColorAttachment(cmdBuf, outImage.vkImage)
        beginRendering(cmdBuf, outImageView.vkImageView, renderArea)
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline)
        nkState.recordRenderActivity(cmdBuf)
        endRendering(cmdBuf)
        transitionViewLayoutToOutput(cmdBuf, outImage.vkImage, outLayout)
        endRender(cmdBuf)
    }


    private fun beginRender(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val beginInfo = VkCommandBufferBeginInfo.calloc(stack)
                .`sType$Default`()
                .flags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
            nkState.prepareRenderActivity()
            vkCheck(vkBeginCommandBuffer(cmdBuf, beginInfo), "Failed to begin command buffer")
        }
    }


    private fun transitionViewLayoutToColorAttachment(cmdBuf: VkCommandBuffer, vkImage: Long) {
        MemoryStack.stackPush().use { stack ->
            val barrier = VkImageMemoryBarrier.calloc(1, stack)
                .`sType$Default`()
                .srcAccessMask(0)
                .dstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                .oldLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .newLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                .image(vkImage)
                .subresourceRange {
                    it
                        .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                        .baseMipLevel(0)
                        .levelCount(1)
                        .baseArrayLayer(0)
                        .layerCount(1)
                }
            vkCmdPipelineBarrier(
                cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                null, null, barrier
            )
        }
    }

    private fun beginRendering(cmdBuf: VkCommandBuffer, vkImageView: Long, renderArea: VkRect2D) {
        MemoryStack.stackPush().use { stack ->
            val colorAttachmentInfo = VkRenderingAttachmentInfo.calloc(1, stack)
                .`sType$Default`()
                .imageView(vkImageView)
                .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
                .clearValue {
                    it.color()
                        .float32(0, 0.1f)
                        .float32(1, 0.2f)
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


    private fun transitionViewLayoutToOutput(cmdBuf: VkCommandBuffer, vkImage: Long, outLayout: Int) {
        MemoryStack.stackPush().use { stack ->
            val barrier = VkImageMemoryBarrier.calloc(1, stack)
                .`sType$Default`()
                .srcAccessMask(0)
                .dstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                .oldLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                .newLayout(outLayout)
                .image(vkImage)
                .subresourceRange {
                    it
                        .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                        .baseMipLevel(0)
                        .levelCount(1)
                        .baseArrayLayer(0)
                        .layerCount(1)
                }
            vkCmdPipelineBarrier(
                cmdBuf,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                null, null, barrier
            )
        }
    }

    private fun endRender(cmdBuf: VkCommandBuffer) {
        vkEndCommandBuffer(cmdBuf)
        nkState.finalizeRenderActivity()
    }
}