package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkImageViewCreateInfo

class ImageView private constructor(val device: Device, val vkImageView: Long, val info: Info) {
    companion object {
        fun create(device: Device, vkImage: Long, info: Info): ImageView {
            MemoryStack.stackPush().use { stack ->
                val lp = stack.mallocLong(1)
                val viewCreateInfo = VkImageViewCreateInfo.calloc(stack)
                    .`sType$Default`()
                    .image(vkImage)
                    .viewType(info.viewType)
                    .format(info.format)
                    .subresourceRange {
                        it.aspectMask(info.aspectMask)
                            .baseMipLevel(0)
                            .levelCount(info.mipLevels)
                            .baseArrayLayer(info.baseArrayLayer)
                            .layerCount(info.layerCount)
                    }
                vkCheck(
                    vkCreateImageView(device.vkDevice, viewCreateInfo, null, lp),
                    "Failed to create image view"
                )
                return ImageView(device, lp[0], info)
            }
        }
    }

    fun cleanup() {
        vkDestroyImageView(device.vkDevice, vkImageView, null)
    }

    data class Info(
        var aspectMask: Int = 0,
        var baseArrayLayer: Int = 0,
        var format: Int = 0,
        var layerCount: Int = 1,
        var mipLevels: Int = 1,
        var viewType: Int = VK_IMAGE_VIEW_TYPE_2D
    ) {
        fun aspectMask(aspectMask: Int): Info {
            this.aspectMask = aspectMask
            return this
        }

        fun baseArrayLayer(baseArrayLayer: Int): Info {
            this.baseArrayLayer = baseArrayLayer
            return this
        }

        fun format(format: Int): Info {
            this.format = format
            return this
        }

        fun layerCount(layerCount: Int): Info {
            this.layerCount = layerCount
            return this
        }

        fun mipLevels(mipLevels: Int): Info {
            this.mipLevels = mipLevels
            return this
        }

        fun viewType(viewType: Int): Info {
            this.viewType = viewType
            return this
        }
    }
}