package gfx.vk

import gfx.Window
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.VK_SUCCESS
import org.tinylog.kotlin.Logger
import java.nio.LongBuffer
import kotlin.math.max
import kotlin.math.min

class Swapchain(device: Device, surface: Surface, window: Window, requestedImages: Int, vsync: Boolean) {
    val device: Device
    val frameInfo: Array<FrameInfo>
    val surfaceFormat: SurfaceFormat
    val swapChainExtent: VkExtent2D
    val vkSwapChain: Long
    var currentFrame: Int
    val numImages: Int get() = frameInfo.size

    init {
        Logger.debug("Creating Vulkan swapchain")
        this.device = device
        MemoryStack.stackPush().use { stack ->
            val physicalDevice = device.physicalDevice
            // Get surface capabilities
            val surfCapabilities = VkSurfaceCapabilitiesKHR.calloc(stack)
            vkCheck(
                KHRSurface.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    device.physicalDevice.vkPhysicalDevice,
                    surface.vkSurface, surfCapabilities
                ), "Failed to get surface capabilities"
            )
            val numImages = calcNumImages(surfCapabilities, requestedImages)
            surfaceFormat = calcSurfaceFormat(physicalDevice, surface)
            swapChainExtent = calcSwapChainExtent(window, surfCapabilities)
            val vkSwapchainCreateInfo = VkSwapchainCreateInfoKHR.calloc(stack)
                .`sType$Default`()
                .surface(surface.vkSurface)
                .minImageCount(numImages)
                .imageFormat(surfaceFormat.imageFormat)
                .imageColorSpace(surfaceFormat.colorSpace)
                .imageExtent(swapChainExtent)
                .imageArrayLayers(1)
                .imageUsage(VK13.VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                .imageSharingMode(VK13.VK_SHARING_MODE_EXCLUSIVE)
                .preTransform(surfCapabilities.currentTransform())
                .compositeAlpha(KHRSurface.VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
                .clipped(true)
            if (vsync) {
                vkSwapchainCreateInfo.presentMode(KHRSurface.VK_PRESENT_MODE_FIFO_KHR)
            } else {
                vkSwapchainCreateInfo.presentMode(KHRSurface.VK_PRESENT_MODE_IMMEDIATE_KHR)
            }
            val lp = stack.mallocLong(1)
            vkCheck(
                KHRSwapchain.vkCreateSwapchainKHR(device.vkDevice, vkSwapchainCreateInfo, null, lp),
                "Failed to create swap chain"
            )
            vkSwapChain = lp[0]

            frameInfo = createFrameInfos(stack, device, vkSwapChain, surfaceFormat.imageFormat)
            currentFrame = 0
        }
    }

    fun cleanup() {
        Logger.debug("Destroying Vulkan swapchain")
        frameInfo.forEach { it.cleanup() }
        KHRSwapchain.vkDestroySwapchainKHR(device.vkDevice, vkSwapChain, null)
    }

    fun acquireNextImage(): Boolean {
        var resize = false
        MemoryStack.stackPush().use { stack ->
            val ip = stack.mallocInt(1)
            val err = KHRSwapchain.vkAcquireNextImageKHR(
                device.vkDevice, vkSwapChain, 0.inv(),
                frameInfo[currentFrame].syncSemaphores.imgAcquisitionSemaphore.vkSemaphore, MemoryUtil.NULL, ip
            )
            if (err == KHRSwapchain.VK_ERROR_OUT_OF_DATE_KHR) {
                resize = true
            } else if (err == KHRSwapchain.VK_SUBOPTIMAL_KHR) {
                resize = true
            } else if (err != VK_SUCCESS) {
                throw RuntimeException("Failed to acquire image: $err")
            }
            currentFrame = ip[0]
        }
        return resize
    }

    fun presentImage(vkQueue: VkQueue): Boolean {
        var resize = false
        MemoryStack.stackPush().use { stack ->
            val present = VkPresentInfoKHR.calloc(stack)
                .`sType$Default`()
                .pWaitSemaphores(
                    stack.longs(
                        frameInfo[currentFrame].syncSemaphores.renderCompleteSemaphore.vkSemaphore
                    )
                )
                .swapchainCount(1)
                .pSwapchains(stack.longs(vkSwapChain))
                .pImageIndices(stack.ints(currentFrame))
            val err = KHRSwapchain.vkQueuePresentKHR(vkQueue, present)
            if (err == KHRSwapchain.VK_ERROR_OUT_OF_DATE_KHR) {
                resize = true
            } else if (err == KHRSwapchain.VK_SUBOPTIMAL_KHR) {
                resize = true
            } else if (err != VK13.VK_SUCCESS) {
                throw RuntimeException("Failed to present KHR: $err")
            }
        }
        currentFrame = (currentFrame + 1) % numImages
        return resize
    }

    private fun createFrameInfos(stack: MemoryStack, device: Device, swapChain: Long, format: Int): Array<FrameInfo> {
        val swapChainImages = getSwapchainImages(stack, device, swapChain)
        val numImages = swapChainImages.capacity()
        val imageViewData = ImageView.Info().format(format).aspectMask(VK13.VK_IMAGE_ASPECT_COLOR_BIT)
        return Array(numImages) {
            FrameInfo(
                swapChainImages[it],
                ImageView.create(device, swapChainImages[it], imageViewData),
                SyncSemaphores.create(device)
            )
        }
    }

    private fun getSwapchainImages(
        stack: MemoryStack,
        device: Device,
        swapChain: Long
    ): LongBuffer {
        val ip = stack.mallocInt(1)
        vkCheck(
            KHRSwapchain.vkGetSwapchainImagesKHR(device.vkDevice, swapChain, ip, null),
            "Failed to get number of surface images"
        )
        val numImages = ip[0]
        val swapChainImages = stack.mallocLong(numImages)
        vkCheck(
            KHRSwapchain.vkGetSwapchainImagesKHR(device.vkDevice, swapChain, ip, swapChainImages),
            "Failed to get surface images"
        )
        return swapChainImages
    }


    companion object {
        private fun calcNumImages(surfCapabilities: VkSurfaceCapabilitiesKHR, requestedImages: Int): Int {
            val maxImages = surfCapabilities.maxImageCount()
            val minImages = surfCapabilities.minImageCount()
            var result = minImages
            if (maxImages != 0) {
                result = min(requestedImages, maxImages)
            }
            result = max(result, minImages)
            Logger.debug(
                "Requested [{}] images, got [{}] images. Surface capabilities, maxImages: [{}], minImages: [{}]",
                requestedImages, result, maxImages, minImages
            )
            return result
        }

        private fun calcSurfaceFormat(physicalDevice: PhysicalDevice, surface: Surface): SurfaceFormat {
            var imageFormat: Int
            var colorSpace: Int
            MemoryStack.stackPush().use { stack ->
                val ip = stack.mallocInt(1)
                vkCheck(
                    KHRSurface.vkGetPhysicalDeviceSurfaceFormatsKHR(
                        physicalDevice.vkPhysicalDevice,
                        surface.vkSurface,
                        ip,
                        null
                    ),
                    "Failed to get number of surface formats"
                )
                val numFormats = ip[0]
                if (numFormats <= 0) {
                    throw RuntimeException("No surface format retrieved")
                }
                val surfaceFormats = VkSurfaceFormatKHR.calloc(numFormats, stack)
                vkCheck(
                    KHRSurface.vkGetPhysicalDeviceSurfaceFormatsKHR(
                        physicalDevice.vkPhysicalDevice,
                        surface.vkSurface,
                        ip,
                        surfaceFormats
                    ),
                    "Failed to get surface formats"
                )
                imageFormat = VK13.VK_FORMAT_B8G8R8A8_SRGB
                colorSpace = surfaceFormats[0].colorSpace()
                for (i in 0 until numFormats) {
                    val surfaceFormatKHR = surfaceFormats[i]
                    if (surfaceFormatKHR.format() == VK13.VK_FORMAT_B8G8R8A8_SRGB &&
                        surfaceFormatKHR.colorSpace() == KHRSurface.VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                    ) {
                        imageFormat = surfaceFormatKHR.format()
                        colorSpace = surfaceFormatKHR.colorSpace()
                        break
                    }
                }
            }
            return SurfaceFormat(imageFormat, colorSpace)
        }

        fun calcSwapChainExtent(
            window: Window,
            surfCapabilities: VkSurfaceCapabilitiesKHR
        ): VkExtent2D {
            val result = VkExtent2D.calloc()
            if (surfCapabilities.currentExtent().width().toLong() == 0xFFFFFFFF) {
                // Surface size undefined. Set to the window size if within bounds
                var width = min(window.width, surfCapabilities.maxImageExtent().width())
                width = max(width, surfCapabilities.minImageExtent().width())
                var height = min(window.height, surfCapabilities.maxImageExtent().height())
                height = max(height, surfCapabilities.minImageExtent().height())
                result.width(width)
                result.height(height)
            } else {
                // Surface already defined, just use that for the swap chain
                result.set(surfCapabilities.currentExtent())
            }
            return result
        }
    }

    data class SurfaceFormat(val imageFormat: Int, val colorSpace: Int)

    data class SyncSemaphores(
        val imgAcquisitionSemaphore: Semaphore,
        val renderCompleteSemaphore: Semaphore
    ) {
        fun cleanup() {
            imgAcquisitionSemaphore.cleanup()
            renderCompleteSemaphore.cleanup()
        }

        companion object {
            fun create(device: Device): SyncSemaphores {
                return SyncSemaphores(Semaphore.create(device), Semaphore.create(device, ))
            }
        }
    }

    data class FrameInfo(val image: Long, val view: ImageView, val syncSemaphores: SyncSemaphores) {
        fun cleanup() {
            view.cleanup()
            syncSemaphores.cleanup()
        }
    }
}