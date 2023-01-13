package gfx.vk

import org.lwjgl.system.MemoryStack
import org.lwjgl.util.vma.Vma
import org.lwjgl.util.vma.Vma.vmaDestroyImage
import org.lwjgl.util.vma.VmaAllocationCreateInfo
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkExtent3D
import org.lwjgl.vulkan.VkImageCreateInfo

class Image private constructor(
    private val device: Device,
    val vkImage: Long,
    val vmaAlloc: Long,
    val info: Info,
) {

    companion object {
        fun create(device: Device, info: Info): Image {
            MemoryStack.stackPush().use { stack ->
                val imageCI = VkImageCreateInfo.calloc(stack)
                    .`sType$Default`()
                    .imageType(VK_IMAGE_TYPE_2D)
                    .format(info.format)
                    .extent(VkExtent3D.malloc(stack).width(info.width).height(info.height).depth(1))
                    .mipLevels(info.mipLevels)
                    .arrayLayers(info.arrayLayers)
                    .samples(info.samples)
                    .tiling(info.tiling)
                    .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT or VK_IMAGE_USAGE_SAMPLED_BIT)
                    .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                    .initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
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
                return Image(device, pImage[0], pAlloc[0], info)
            }
        }
    }

    fun cleanup() {
        vmaDestroyImage(device.memoryAllocator.vmaAllocator, vkImage, vmaAlloc)
    }

    data class Info(
        val width: Int, val height: Int, val channels: Int, val format: Int,
        val mipLevels: Int = 1, val arrayLayers: Int = 1,
        val samples: Int = VK_SAMPLE_COUNT_1_BIT, val tiling: Int = VK_IMAGE_TILING_OPTIMAL
    )
}