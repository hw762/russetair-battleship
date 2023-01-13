package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkCommandBuffer
import org.lwjgl.vulkan.VkCommandBufferAllocateInfo
import org.lwjgl.vulkan.VkCommandPoolCreateInfo

class CommandPool(val device: Device, val vkCommandPool: Long) {
    companion object {
        fun create(device: Device, queueFamilyIndex: Int): CommandPool {
            MemoryStack.stackPush().use { stack ->
                val commandPoolCI = VkCommandPoolCreateInfo.calloc(stack)
                    .`sType$Default`()
                    .flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
                    .queueFamilyIndex(queueFamilyIndex)

                val lp = stack.mallocLong(1)
                vkCheck(vkCreateCommandPool(device.vkDevice, commandPoolCI, null, lp),
                    "Failed to create command pool")
                return CommandPool(device, lp[0])
            }
        }
    }

    fun allocate(count: Int, primary: Boolean = true): Array<VkCommandBuffer> {
        MemoryStack.stackPush().use { stack ->
            val allocInfo = VkCommandBufferAllocateInfo.calloc(stack)
                .`sType$Default`()
                .commandPool(vkCommandPool)
                .level(if (primary) VK_COMMAND_BUFFER_LEVEL_PRIMARY else VK_COMMAND_BUFFER_LEVEL_SECONDARY)
                .commandBufferCount(count)
            val pCmdBufs = stack.mallocPointer(count)
            vkCheck(vkAllocateCommandBuffers(device.vkDevice, allocInfo, pCmdBufs),
                "Failed to allocate command buffers")
            return Array(count) {
                VkCommandBuffer(pCmdBufs[it], device.vkDevice)
            }
        }
    }

    fun cleanup() {
        vkDestroyCommandPool(device.vkDevice, vkCommandPool, null)
    }
}