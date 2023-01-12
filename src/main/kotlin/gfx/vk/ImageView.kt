package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkComponentMapping
import org.lwjgl.vulkan.VkImageSubresourceRange
import org.lwjgl.vulkan.VkImageViewCreateInfo

class ImageView(private val device: Device, val image: Image, format: Int) {
    val vkImageView: Long

    init {
        MemoryStack.stackPush().use { stack ->
            val ci = VkImageViewCreateInfo.calloc(stack)
                .`sType$Default`()
                .image(image.vkImage)
                .viewType(VK_IMAGE_VIEW_TYPE_2D)
                .format(format)
                .components(VkComponentMapping.calloc(stack)
                    .r(VK_COMPONENT_SWIZZLE_R)
                    .g(VK_COMPONENT_SWIZZLE_G)
                    .b(VK_COMPONENT_SWIZZLE_B)
                    .a(VK_COMPONENT_SWIZZLE_A))
                .subresourceRange(VkImageSubresourceRange.calloc(stack)
                    .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                    .layerCount(1)
                    .baseArrayLayer(0)
                    .levelCount(1)
                    .baseMipLevel(0))
            val lp = stack.mallocLong(1)
            vkCheck(vkCreateImageView(device.vkDevice, ci, null, lp),
                "Failed to create image view")
            vkImageView = lp[0]
        }
    }

    fun cleanup() {
        vkDestroyImageView(device.vkDevice, vkImageView, null)
    }
}