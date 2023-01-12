import gfx.Window
import gfx.vk.Device
import gfx.vk.Instance
import gfx.vk.PhysicalDevice

fun main() {
    val window = Window("Russetair Battleship")
    val instance = Instance(true)
    val physicalDevice = PhysicalDevice.createPhysicalDevice(instance)
    val device = Device(instance, physicalDevice)
}
