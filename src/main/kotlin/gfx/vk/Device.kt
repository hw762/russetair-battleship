package gfx.vk

import gfx.vk.VulkanUtils.Companion.stringsToPointerBuffer
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.KHRDynamicRendering.VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
import org.lwjgl.vulkan.KHRPortabilitySubset.VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
import org.lwjgl.vulkan.KHRSwapchain.VK_KHR_SWAPCHAIN_EXTENSION_NAME
import org.lwjgl.vulkan.VK10.vkCreateDevice
import org.tinylog.kotlin.Logger

class Device(instance: Instance, physicalDevice: PhysicalDevice) {
    val physicalDevice: PhysicalDevice
    val vkDevice: VkDevice
    val memoryAllocator: MemoryAllocator
    val samplerAnisotropy: Boolean
    init {
        Logger.debug("Creating device")
        this.physicalDevice = physicalDevice
        MemoryStack.stackPush().use { stack ->
            val supportedFeatures = physicalDevice.vkPhysicalDeviceFeatures
            samplerAnisotropy = supportedFeatures.samplerAnisotropy()
            val deviceCreateInfo = Companion.deviceCreateInfo(this, stack, supportedFeatures, physicalDevice)
            val pp = stack.mallocPointer(1)
            vkCheck(
                vkCreateDevice(physicalDevice.vkPhysicalDevice, deviceCreateInfo, null, pp),
                "Failed to create device")
            vkDevice = VkDevice(pp.get(0), physicalDevice.vkPhysicalDevice, deviceCreateInfo)
            memoryAllocator = MemoryAllocator(instance, physicalDevice, vkDevice)
        }
    }

    fun cleanup() {
        Logger.debug("Destroying Vulkan device")
        memoryAllocator.cleanup()
        VK13.vkDestroyDevice(vkDevice, null)
    }

    fun waitIdle() {
        VK13.vkDeviceWaitIdle(vkDevice)
    }

    companion object {
        private fun deviceCreateInfo(
            device: Device, stack: MemoryStack,
            supportedFeatures: VkPhysicalDeviceFeatures,
            physicalDevice: PhysicalDevice
        ): VkDeviceCreateInfo {
            // Required extensions
            val extensions = listOf(
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
            )
            val requiredExtensions = stringsToPointerBuffer(stack, extensions)
            // Set up required features
            val dynamicRendering = VkPhysicalDeviceDynamicRenderingFeatures.calloc(stack)
                .`sType$Default`()
                .dynamicRendering(true)
            val features = VkPhysicalDeviceFeatures.calloc(stack)
                .samplerAnisotropy(device.samplerAnisotropy)
                .depthClamp(supportedFeatures.depthClamp())
            // Enable all queue families
            val queuePropsBuff = physicalDevice.vkQueueFamilyProps
            val numQueueFamilies = queuePropsBuff.capacity()
            val queueCreationInfoBuf = VkDeviceQueueCreateInfo.calloc(numQueueFamilies, stack)
            for (i in 0 until numQueueFamilies) {
                val priorities = stack.callocFloat(queuePropsBuff.get(i).queueCount())
                queueCreationInfoBuf[i]
                    .`sType$Default`()
                    .queueFamilyIndex(i)
                    .pQueuePriorities(priorities)
            }
            return VkDeviceCreateInfo.calloc(stack)
                .`sType$Default`()
                .pNext(dynamicRendering)
                .ppEnabledExtensionNames(requiredExtensions)
                .pEnabledFeatures(features)
                .pQueueCreateInfos(queueCreationInfoBuf)
        }
    }
}