package gfx

import gfx.vk.ImageView
import org.lwjgl.vulkan.VkCommandBuffer
import org.lwjgl.vulkan.VkRect2D

interface Renderer {
    /**
     * Renders to an output image view using the unstarted command buffer.
     */
    fun render(cmdBuf: VkCommandBuffer, outImageView: ImageView, outLayout: Int, renderArea: VkRect2D)
}