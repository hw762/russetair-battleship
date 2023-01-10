package hw762.russetair.vk

import org.joml.Matrix4f
import org.lwjgl.PointerBuffer
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.vulkan.VK13.*
import org.lwjgl.vulkan.VkCommandBuffer

class VulkanUtils {
    companion object {
        fun vkCheck(err: Int, errMsg: String) {
            if (err != VK_SUCCESS) {
                throw RuntimeException("$errMsg: $err")
            }
        }
        fun stringsToPointerBuffer(stack: MemoryStack, ss: List<String>): PointerBuffer {
            val pb = stack.mallocPointer(ss.size)
            for (s in ss) {
                val cstr = stack.UTF8(s)
                pb.put(cstr)
            }
            pb.flip()
            return pb
        }
    }
}