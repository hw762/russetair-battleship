#include "pipeline.h"

#include "device.h"
#include "swapchain.h"
#include "vk.h"

#include <flecs.h>
#include <stdlib.h>

// Pipeline
// newPipelineClearSwapchain(const Swapchain* swapchain)
// {
//     VkDevice device; // TODO
//     VkPipelineCache cache; // TODO
//     VkPipelineRenderingCreateInfoKHR prci = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
//         .colorAttachmentCount = 1,
//         .pColorAttachmentFormats = swapchain->format.imageFormat
//     };
//     VkGraphicsPipelineCreateInfo ci[1] = {
//         {
//             .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//         }
//     };
//     VkPipeline pipelines[1];
//     vkCheck(vkCreateGraphicsPipelines(device, cache, 1, ci, NULL, pipelines))
//     {
//         ecs_abort(1, "Failed to create graphics pipeline");
//     }

// }
