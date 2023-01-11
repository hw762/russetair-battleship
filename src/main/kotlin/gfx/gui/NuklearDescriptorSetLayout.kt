package gfx.gui

import gfx.vk.DescriptorSetLayout
import gfx.vk.Device
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkDescriptorSetLayoutBinding
import org.lwjgl.vulkan.VkDescriptorSetLayoutCreateInfo

class NuklearDescriptorSetLayout(device: Device) : DescriptorSetLayout(device) {
    override val vkDescriptorLayout: Long

    init {
        MemoryStack.stackPush().use { stack ->
            val bindings = VkDescriptorSetLayoutBinding.calloc(1)
            bindings[0]
                .binding(0)
                .descriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                .descriptorCount(1)
                .stageFlags(VK_SHADER_STAGE_VERTEX_BIT)
            val ci = VkDescriptorSetLayoutCreateInfo.calloc(stack)
                .`sType$Default`()
                .pBindings(bindings)
            val lp = stack.mallocLong(1)
            vkCheck(
                vkCreateDescriptorSetLayout(device.vkDevice, ci, null, lp),
                "Failed to create descriptor layout"
            )
            vkDescriptorLayout = lp[0]
        }
    }
}
