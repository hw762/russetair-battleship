package gfx.vk

import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.util.vma.Vma.*
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*
import org.tinylog.kotlin.Logger
import java.nio.ByteBuffer

/**
 * A regular 2D image texture on a single queue
 */
class VulkanTexture(
    private val device: Device,
    val width: Int, val height: Int, val channels: Int, val format: Int,
    val mipLevels: Int, val arrayLayers: Int, val samples: Int, val tiling: Int
) : UploadActivity {
    val image: Image

    private var loaded: Boolean = false
        private set
    private var stagingBuf: VulkanBuffer? = null

    init {
        Logger.debug("Creating VulkanTexture")
        image = Image(device, width, height, channels, format, mipLevels, arrayLayers, samples, tiling)
    }

    fun cleanup() {
        cleanupStagingBuffer()
        image.cleanup()
    }

    /**
     * Allocate and map staging buffer
     */
    fun prepareStagingBuffer() {
        assert(!loaded) { "Texture already loaded" }
        loaded = false
        if (stagingBuf != null) {
            return
        }
        val size = width.toLong() * height.toLong() * channels.toLong()
        stagingBuf = VulkanBuffer(
            device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        )
    }

    /**
     * Load pixel data to staging buffer
     */
    fun loadPixels(pixels: ByteBuffer) {
        assert(!loaded && stagingBuf != null)
        val size = width.toLong() * height.toLong() * channels.toLong()
        assert(size <= pixels.remaining())
        // Copy to staging buffer
        MemoryUtil.memByteBuffer(stagingBuf!!.map(), size.toInt()).put(pixels)
        stagingBuf!!.unmap()
    }

    fun view(): ImageView {
        return ImageView(device, image.vkImage, ImageView.Info().format(format))
    }

    override fun recordUpload(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val barrier = VkImageMemoryBarrier.calloc(1, stack)
                .`sType$Default`()
                .srcAccessMask(0)
                .dstAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT)
                .oldLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .newLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                .image(image.vkImage)
                .subresourceRange(
                    VkImageSubresourceRange.calloc(stack)
                        .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                        .baseMipLevel(0)
                        .levelCount(1)
                        .baseArrayLayer(0)
                        .layerCount(1)
                )
            vkCmdPipelineBarrier(
                cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0, null, null, barrier
            )
            val copyRegion = VkBufferImageCopy.calloc(1, stack)
                .imageSubresource(
                    VkImageSubresourceLayers.calloc(stack)
                        .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                        .baseArrayLayer(0)
                        .layerCount(1)
                        .mipLevel(0)
                )
                .imageExtent(VkExtent3D.calloc(stack).width(width).height(height).depth(1))
            vkCmdCopyBufferToImage(
                cmdBuf, stagingBuf!!.buffer, image.vkImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyRegion
            )
            barrier
                .srcAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT)
                .dstAccessMask(VK_ACCESS_SHADER_READ_BIT)
                .oldLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                .newLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            vkCmdPipelineBarrier(
                cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                null, null, barrier
            )
        }
    }

    /**
     * Finalize after record
     */
    override fun finalizeAfterUpload() {
        cleanupStagingBuffer()
    }

    private fun cleanupStagingBuffer() {
        if (stagingBuf == null) {
            return
        }
        stagingBuf!!.cleanup()
        stagingBuf = null
    }
}