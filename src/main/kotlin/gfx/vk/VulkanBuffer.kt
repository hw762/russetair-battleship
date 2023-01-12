package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.PointerBuffer
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.util.vma.Vma
import org.lwjgl.util.vma.VmaAllocationCreateInfo
import org.lwjgl.vulkan.VK10.VK_SHARING_MODE_EXCLUSIVE
import org.lwjgl.vulkan.VkBufferCreateInfo

class VulkanBuffer(device: Device, size: Long, bufferUsage: Int,
                   vmaMemoryUsage: Int, vmaAllocRequiredFlags: Int) {
    val allocation: Long
    val buffer: Long
    val device: Device
    val pb: PointerBuffer
    val requestedSize: Long

    var mappedMemory: Long
    init {
        this.device = device
        requestedSize = size
        mappedMemory = MemoryUtil.NULL
        MemoryStack.stackPush().use { stack ->
            val bufferCreateInfo = VkBufferCreateInfo.calloc(stack)
                .`sType$Default`()
                .size(size)
                .usage(bufferUsage)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            val allocInfo = VmaAllocationCreateInfo.calloc(stack)
                .requiredFlags(vmaAllocRequiredFlags)
                .usage(vmaMemoryUsage)
            val pAllocation = stack.callocPointer(1)
            val lp = stack.mallocLong(1)
            // TODO: support MAPPED bit to map on create
            vkCheck(
                Vma.vmaCreateBuffer(
                    device.memoryAllocator.vmaAllocator, bufferCreateInfo, allocInfo, lp,
                    pAllocation, null
                ),
                "Failed to create buffer")
            buffer = lp[0]
            allocation = pAllocation[0]
            pb = MemoryUtil.memAllocPointer(1)
        }
    }
    fun cleanup() {
        MemoryUtil.memFree(pb)
        unmap()
        Vma.vmaDestroyBuffer(device.memoryAllocator.vmaAllocator, buffer, allocation)
    }

    fun map(): Long {
        if (mappedMemory == MemoryUtil.NULL) {
            vkCheck(
                Vma.vmaMapMemory(device.memoryAllocator.vmaAllocator, allocation, pb),
                "Failed to map buffer")
            mappedMemory = pb[0]
        }
        return mappedMemory
    }
    fun unmap() {
        if (mappedMemory != MemoryUtil.NULL) {
            Vma.vmaUnmapMemory(device.memoryAllocator.vmaAllocator, allocation)
            mappedMemory = MemoryUtil.NULL
        }
    }
}