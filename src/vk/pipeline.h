#pragma once

#include <vulkan/vulkan.h>

struct Device;
typedef struct Device Device;

struct Queue;
typedef struct Queue Queue;

struct ImageView;
typedef struct ImageView ImageView;

typedef struct Pipeline {
    VkPipeline handle;
} Pipeline;

// Pipeline
// newPipelineClearSwapchain(const Swapchain* swapchain);
