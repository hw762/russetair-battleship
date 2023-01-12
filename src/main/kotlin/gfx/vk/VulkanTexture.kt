package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.util.vma.Vma.*
import org.lwjgl.util.vma.VmaAllocationCreateInfo
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*
import org.tinylog.kotlin.Logger
import java.nio.ByteBuffer

class VulkanTexture(
    private val device: Device,
    val width: Int, val height: Int, val channels: Int, val format: Int,
    val mipLevels: Int, val arrayLayers: Int, val samples: Int, val tiling: Int
) {
    val vkImage: Long
    val vmaAlloc: Long
    var loaded: Boolean = false
        private set
    private var stagingBuf: VulkanBuffer? = null

    init {
        Logger.debug("Creating VulkanTexture")
        MemoryStack.stackPush().use { stack ->
            val imageCI = VkImageCreateInfo.calloc(stack)
                .`sType$Default`()
                .imageType(VK_IMAGE_TYPE_2D)
                .format(format)
                .extent(VkExtent3D.malloc(stack).width(width).height(height))
                .mipLevels(mipLevels)
                .arrayLayers(arrayLayers)
                .samples(samples)
                .tiling(tiling)
                .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT or VK_IMAGE_USAGE_SAMPLED_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            val allocCI = VmaAllocationCreateInfo.calloc(stack)
                .usage(VMA_MEMORY_USAGE_AUTO)
            val pImage = stack.mallocLong(1)
            val pAlloc = stack.mallocPointer(1)
            vkCheck(
                vmaCreateImage(
                    device.memoryAllocator.vmaAllocator,
                    imageCI, allocCI, pImage, pAlloc, null
                ),
                "Failed to create image"
            )
            vkImage = pImage[0]
            vmaAlloc = pAlloc[0]
        }
    }

    fun cleanup() {
        finalizeAfterSubmit()
        vmaDestroyImage(device.memoryAllocator.vmaAllocator, vkImage, vmaAlloc)
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

    fun recordCommands(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
            val barrier = VkImageMemoryBarrier.calloc(1, stack)
                .`sType$Default`()
                .srcAccessMask(0)
                .dstAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT)
                .oldLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .newLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                .image(vkImage)
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
                cmdBuf, stagingBuf!!.buffer, vkImage,
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
    fun finalizeAfterSubmit() {
        if (stagingBuf == null) {
            return
        }
        stagingBuf!!.cleanup()
        stagingBuf = null
    }

    fun view(): ImageView {
        return ImageView(device, vkImage, format)
    }
}