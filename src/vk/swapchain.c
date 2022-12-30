#include "swapchain.h"
#include "vk.h"

ecs_entity_t createSwapchain(ecs_world_t* ecs, ecs_entity_t eSystem, int requestedImages, bool vSync)
{
    ecs_trace("Creating Swapchain");
    ecs_log_push();

    VkSurfaceKHR surface = *ecs_get(ecs, eSystem, VkSurfaceKHR);
    VkPhysicalDevice physDev = *ecs_get(ecs, eSystem, SelectedPhysicalDevice);

    ecs_log_pop();
    return 0;
}
