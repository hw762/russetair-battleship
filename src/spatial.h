#pragma once

#include <flecs.h>

/// @brief 3D position
typedef struct {
    float x, y, z;
} Position;

/// @brief Rotation quaternion
typedef struct {
    float x, y, z, w;
} Rotation;

/// @brief Velocity in m/s
typedef struct {
    float x, y, z;
} Velocity;

/// @brief Angular velocity in rad/s
typedef struct {
    float x, y, z;
} AngularVelocity;

/// @brief Acceleration in m/s^2
typedef struct {
    float x, y, z;
} Acceleration;

void spatial_register(ecs_world_t* ecs);