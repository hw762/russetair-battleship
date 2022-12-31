#pragma once

#include <flecs.h>

#include "spatial.h"

/// @brief Registers types for players, spectators, etc
/// @param ecs
void player_register(ecs_world_t* ecs);

/// @brief Spawn a spectator at location
/// @param ecs
/// @param pos
/// @param rot
void spectator_spawn(ecs_world_t* ecs, Position pos, Rotation rot);