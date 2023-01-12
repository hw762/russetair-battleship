import gfx.Window
import gfx.vk.Device
import gfx.vk.Instance
import gfx.vk.PhysicalDevice
import org.lwjgl.glfw.GLFW

fun main() {
    val window = Window("Russetair Battleship")
    val instance = Instance(true)
    val physicalDevice = PhysicalDevice.createPhysicalDevice(instance)
    val device = Device(instance, physicalDevice)

    while (!window.shouldClose()) {
        window.pollEvents()
        if (window.isKeyPressed(GLFW.GLFW_KEY_ESCAPE)) {
            GLFW.glfwSetWindowShouldClose(window.windowHandle, true)
        }
    }
    device.cleanup()
    physicalDevice.cleanup()
    instance.cleanup()
    window.cleanup()
}
