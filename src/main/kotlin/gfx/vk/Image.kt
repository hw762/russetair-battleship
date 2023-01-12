package gfx.vk

import org.lwjgl.system.MemoryStack
import org.lwjgl.util.vma.Vma
import org.lwjgl.util.vma.Vma.vmaDestroyImage
import org.lwjgl.util.vma.VmaAllocationCreateInfo
import org.lwjgl.vulkan.VK10
import org.lwjgl.vulkan.VkCommandBuffer
import org.lwjgl.vulkan.VkExtent3D
import org.lwjgl.vulkan.VkImageCreateInfo
import org.lwjgl.vulkan.VkImageMemoryBarrier

class Image(
    private val device: Device,
    val width: Int, val height: Int, val channels: Int, val format: Int,
    val mipLevels: Int, val arrayLayers: Int, val samples: Int, val tiling: Int
) {
    val vkImage: Long
    val vmaAlloc: Long

    init {
        MemoryStack.stackPush().use { stack ->
            val imageCI = VkImageCreateInfo.calloc(stack)
                .`sType$Default`()
                .imageType(VK10.VK_IMAGE_TYPE_2D)
                .format(format)
                .extent(VkExtent3D.malloc(stack).width(width).height(height))
                .mipLevels(mipLevels)
                .arrayLayers(arrayLayers)
                .samples(samples)
                .tiling(tiling)
                .usage(VK10.VK_IMAGE_USAGE_TRANSFER_DST_BIT or VK10.VK_IMAGE_USAGE_SAMPLED_BIT)
                .sharingMode(VK10.VK_SHARING_MODE_EXCLUSIVE)
                .initialLayout(VK10.VK_IMAGE_LAYOUT_UNDEFINED)
            val allocCI = VmaAllocationCreateInfo.calloc(stack)
                .usage(Vma.VMA_MEMORY_USAGE_AUTO)
            val pImage = stack.mallocLong(1)
            val pAlloc = stack.mallocPointer(1)
            VulkanUtils.vkCheck(
                Vma.vmaCreateImage(
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
        vmaDestroyImage(device.memoryAllocator.vmaAllocator, vkImage, vmaAlloc)
    }

    fun view(): ImageView {
        return ImageView(device, this, format)
    }
}