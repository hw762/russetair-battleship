package gfx.vk

import org.lwjgl.glfw.GLFWVulkan
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.KHRSurface
import org.tinylog.kotlin.Logger

class Surface private constructor(
    private val physicalDevice: PhysicalDevice,
    val vkSurface: Long
) {
    companion object {
        fun create(physicalDevice: PhysicalDevice, windowHandle: Long): Surface {
            Logger.debug("Creating Vulkan surface")
            MemoryStack.stackPush().use { stack ->
                val pSurface = stack.mallocLong(1)
                GLFWVulkan.glfwCreateWindowSurface(
                    physicalDevice.vkPhysicalDevice.instance,
                    windowHandle, null, pSurface
                )
                return Surface(physicalDevice, pSurface[0])
            }
        }
    }

    init {

    }
    fun cleanup() {
        Logger.debug("Destroying Vulkan surface")
        KHRSurface.vkDestroySurfaceKHR(physicalDevice.vkPhysicalDevice.instance, vkSurface, null)
    }
}