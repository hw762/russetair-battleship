package gfx.vk

import org.lwjgl.vulkan.VkCommandBuffer

interface CommandRecordable {
    /**
     * Perform any state changes required prior to a command buffer record. E.g. set up buffers
     */
    fun prepareRecord()

    /**
     * Record command buffer
     */
    fun recordCommands(cmdBuf: VkCommandBuffer)

    /**
     * Perform any state changes required after command buffer is submitted. E.g. clean up allocations
     */
    fun finalizeAfterSubmit()
}