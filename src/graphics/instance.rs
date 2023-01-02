use std::sync::Arc;
use vulkano::device::{Device, DeviceCreateInfo, DeviceExtensions, Features, Queue, QueueCreateInfo};
use vulkano::instance::{Instance, InstanceCreateInfo};
use vulkano::swapchain::Surface;
use vulkano::{Version, VulkanLibrary};
use vulkano::device::physical::{PhysicalDevice, PhysicalDeviceType};

pub struct VulkanInstance {
    pub instance: Arc<Instance>,
}

impl VulkanInstance {
    pub fn new() -> Self {
        let library = VulkanLibrary::new().unwrap();
        let required_extensions = vulkano_win::required_extensions(&library);
        println!("Supported API: {:?}", library.api_version());

        // Now creating the instance.
        let instance = Instance::new(
            library,
            InstanceCreateInfo {
                enabled_extensions: required_extensions,
                // Enable enumerating devices that use non-conformant vulkan implementations. (ex. MoltenVK)
                enumerate_portability: true,
                ..Default::default()
            },
        )
            .unwrap();

         Self {
             instance,
         }
    }
}