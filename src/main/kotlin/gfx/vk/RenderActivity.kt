package gfx.vk

import org.lwjgl.vulkan.VkCommandBuffer

interface RenderActivity {
    /**
     * Perform any state changes required prior to a command buffer record. E.g. set up buffers
     */
    fun prepareRenderActivity()
    /**
     * Record command buffer
     */
    fun recordRenderActivity(cmdBuf: VkCommandBuffer)

    /**
     * Perform any state changes required after command buffer is submitted. E.g. clean up allocations
     */
    fun finalizeRenderActivity  ()
}