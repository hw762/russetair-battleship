#include "queue.h"
#include "vk.h"

#include <flecs.h>
#include <stdlib.h>

#include "command_buffer.h"
#include "device.h"
#include "physical_device.h"

void queueSubmit(const Queue* queue, const CommandBuffer* cmdBuf,
    VkSemaphore waitSemaphore, VkPipelineStageFlags dstStageMask,
    VkSemaphore signalSemaphore, VkFence fence)
{
    ecs_trace("Submitting to device queue...");
    ecs_log_push();
    VkSubmitInfo submit
        = {
              .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
              .commandBufferCount = 1,
              .pCommandBuffers = &cmdBuf->handle,
              .waitSemaphoreCount = 1,
              .pWaitSemaphores = &waitSemaphore,
              .signalSemaphoreCount = 1,
              .pSignalSemaphores = &signalSemaphore,
              .pWaitDstStageMask = &dstStageMask,
          };
    vkCheck(vkQueueSubmit(queue->handle, 1, &submit, fence))
    {
        ecs_abort(1, "Failed to submit queue");
    }
    ecs_trace("Done submitting to device queue");
    ecs_log_pop();
}