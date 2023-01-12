import gfx.Window
import gfx.vk.*
import org.lwjgl.glfw.GLFW

fun main() {
    val window = Window("Russetair Battleship")
    val instance = Instance(true)
    val physicalDevice = PhysicalDevice.createPhysicalDevice(instance)
    val surface = Surface(physicalDevice, window.windowHandle)
    val device = Device(instance, physicalDevice)
    val swapchain = Swapchain(device, surface, window, 3, true)
    while (!window.shouldClose()) {
        window.pollEvents()
        if (window.isKeyPressed(GLFW.GLFW_KEY_ESCAPE)) {
            break // Exit
        }
    }
    swapchain.cleanup()
    device.cleanup()
    surface.cleanup()
    physicalDevice.cleanup()
    instance.cleanup()
    window.cleanup()
}
