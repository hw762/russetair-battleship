import gfx.Window
import gfx.gui.NuklearRenderer
import gfx.vk.*
import org.lwjgl.glfw.GLFW
import org.lwjgl.system.MemoryStack
import org.lwjgl.vulkan.KHRSwapchain
import org.lwjgl.vulkan.KHRSwapchain.VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
import org.lwjgl.vulkan.VK10
import org.lwjgl.vulkan.VK10.vkQueueWaitIdle
import org.lwjgl.vulkan.VkQueue
import org.lwjgl.vulkan.VkRect2D

fun main() {
    val window = Window("Russetair Battleship")
    val instance = Instance(true)
    val physicalDevice = PhysicalDevice.createPhysicalDevice(instance)
    val surface = Surface.create(physicalDevice, window.windowHandle)
    val device = Device(instance, physicalDevice)
    var swapchain = Swapchain(device, surface, window, 3, true)
    val pipelineCache = PipelineCache(device)
    val presentQFI = device.getPresentQueueFamilyIndex(surface)
    val presentQueue = device.getQueue(presentQFI)
    val commandPool = CommandPool.create(device, presentQFI)
    val nkRenderer = NuklearRenderer(device, pipelineCache, swapchain.surfaceFormat.imageFormat)

    while (!window.shouldClose()) {
        window.pollEvents()
        if (window.isKeyPressed(GLFW.GLFW_KEY_ESCAPE)) {
            break // Exit
        }
        MemoryStack.stackPush().use { stack ->
            val cmdBufs = commandPool.allocate(1, true)
            if (window.isResized() || swapchain.acquireNextImage()) {
                window.resetResized()
                device.waitIdle()
                vkQueueWaitIdle(presentQueue)
                swapchain.cleanup()
                swapchain = Swapchain(device, surface, window, 3, true)
                swapchain.acquireNextImage()
            }
            val frame = swapchain.frameInfo[swapchain.currentFrame]
            val renderArea = VkRect2D.calloc(stack)
                .extent(swapchain.swapChainExtent)
            nkRenderer.render(cmdBufs[0], frame.image, frame.view, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, renderArea)
            if (swapchain.presentImage(presentQueue)) {
                window.setResized(true)
            }
        }
    }
    device.waitIdle()
    swapchain.cleanup()
    device.cleanup()
    surface.cleanup()
    physicalDevice.cleanup()
    instance.cleanup()
    window.cleanup()
}