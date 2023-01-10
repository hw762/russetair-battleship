package gfx.gui

import gfx.vk.DescriptorSetLayout
import gfx.vk.Device

class NuklearDescriptorSetLayout(device: Device) : DescriptorSetLayout(device) {
    override val vkDescriptorLayout: Long

}
