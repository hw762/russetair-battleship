package gfx.vk

import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.util.vma.Vma
import org.lwjgl.util.vma.VmaAllocatorCreateInfo
import org.lwjgl.util.vma.VmaVulkanFunctions
import org.lwjgl.vulkan.VkDevice

class MemoryAllocator(instance: Instance, physicalDevice: PhysicalDevice, vkDevice: VkDevice) {
    val vmaAllocator: Long

    init {
        MemoryStack.stackPush().use { stack ->
            val pAllocator = stack.mallocPointer(1)
            val vmaVulkanFunctions = VmaVulkanFunctions.calloc(stack)
                .set(instance.vkInstance, vkDevice)
            val createInfo = VmaAllocatorCreateInfo.calloc(stack)
                .instance(instance.vkInstance)
                .device(vkDevice)
                .physicalDevice(physicalDevice.vkPhysicalDevice)
                .pVulkanFunctions(vmaVulkanFunctions)
            vkCheck(Vma.vmaCreateAllocator(createInfo, pAllocator), "Failed to create VMA allocator")
            vmaAllocator = pAllocator[0]
        }
    }

    fun cleanup() {
        Vma.vmaDestroyAllocator(vmaAllocator)
    }
}