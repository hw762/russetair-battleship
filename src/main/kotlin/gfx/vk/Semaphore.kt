package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.vkCreateSemaphore
import org.lwjgl.vulkan.VK10.vkDestroySemaphore
import org.lwjgl.vulkan.VkSemaphoreCreateInfo

class Semaphore private constructor(val device: Device, val vkSemaphore: Long) {
    fun cleanup() {
        vkDestroySemaphore(device.vkDevice, vkSemaphore, null)
    }

    companion object {
        fun create(device: Device): Semaphore {
            MemoryStack.stackPush().use { stack ->
                val semaphoreCreateInfo = VkSemaphoreCreateInfo.calloc(stack)
                    .`sType$Default`()
                val lp = stack.mallocLong(1)
                vkCheck(
                    vkCreateSemaphore(device.vkDevice, semaphoreCreateInfo, null, lp),
                    "Failed to create semaphore")
                return Semaphore(device, lp[0])
            }
        }
    }
}