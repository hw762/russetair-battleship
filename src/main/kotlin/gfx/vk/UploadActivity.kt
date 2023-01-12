package gfx.vk

import org.lwjgl.vulkan.VkCommandBuffer

interface UploadActivity {
    fun recordUpload(cmdBuf: VkCommandBuffer)

    fun finalizeAfterUpload()
}