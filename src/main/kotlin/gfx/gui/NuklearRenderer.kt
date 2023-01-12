package gfx.gui


import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.nuklear.*
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.system.MemoryUtil.nmemAllocChecked
import org.lwjgl.system.MemoryUtil.nmemFree
import org.lwjgl.util.vma.*
import org.lwjgl.util.vma.Vma.*
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.*


class NuklearRenderer(val device: Device, val pipelineCache: PipelineCache, val colorFormat: Int) {

    private val nkState = NuklearState(device)
    private val pipeline = NuklearPipeline(device, pipelineCache, colorFormat)

    init {
        MemoryStack.stackPush().use { stack ->

        }
    }

    fun cleanup() {
        pipeline.cleanup()
        nkState.cleanup()
    }

    fun render(cmdBuf: VkCommandBuffer) {
        MemoryStack.stackPush().use { stack ->
        }
    }





    companion object {




    }
}