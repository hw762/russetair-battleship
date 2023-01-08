#pragma once

#define vkIfFailed(stmt) if ((stmt) != VK_SUCCESS)

#define vkCheck(stmt, ...)            \
    do {                              \
        if ((stmt) != VK_SUCCESS) {   \
            ecs_abort(1, __VA_ARGS__) \
        }                             \
    } while (0)