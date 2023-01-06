#include "image.h"

#include <flecs.h>
#include <stdlib.h>
#include <vk_mem_alloc.h>

#include "check.h"
#include "device.h"
#include "physical_device.h"

Image newImage(const Device* pDevice, const ImageInfo* pInfo, const void* pData, VkDeviceSize size)
{

    return (Image) {
        .handle = 0,
        .pDevice = 0,
        .info = 0,
    };
}
