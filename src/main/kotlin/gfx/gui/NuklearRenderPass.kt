package gfx.gui

import gfx.vk.Device
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*

class NuklearRenderPass(private val device: Device, private val colorFormat: Int) {
    val vkRenderPass: Long

    init {
        MemoryStack.stackPush().use { stack ->
            val attachmentDescriptions = VkAttachmentDescription.calloc(1, stack)
            attachmentDescriptions[0]
                .format(colorFormat)
                .samples(VK_SAMPLE_COUNT_1_BIT)
                .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
                .stencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
                .stencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
                .initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .finalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            val subpassDependendcies = VkSubpassDependency.calloc(1, stack)
            subpassDependendcies[0]
                .srcSubpass(VK_SUBPASS_EXTERNAL)
                .dstSubpass(0)
                .srcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                .dstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                .srcAccessMask(0)
                .dstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                .dependencyFlags(0)
            val colorAttachments = VkAttachmentReference.calloc(1, stack)
            colorAttachments[0].attachment(0).layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)

            val subpasses = VkSubpassDescription.calloc(1, stack)
            subpasses[0]
                .pipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
                .pColorAttachments(colorAttachments)

            val ci = VkRenderPassCreateInfo.calloc(stack)
                .`sType$Default`()
                .pAttachments(attachmentDescriptions)
                .pDependencies(subpassDependendcies)
                .pSubpasses(subpasses)
            val lp = stack.mallocLong(1)
            vkCheck(
                vkCreateRenderPass(device.vkDevice, ci, null, lp),
                "Failed to create render pass"
            )
            vkRenderPass = lp[0]
        }
    }
    fun cleanup() {
        vkDestroyRenderPass(device.vkDevice, vkRenderPass, null)
    }
}
