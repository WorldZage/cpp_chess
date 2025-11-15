#pragma once

#include <SDL3/SDL.h>
#include <entt/entt.hpp>

#include <glm/glm.hpp> // core
#include <glm/gtc/constants.hpp>
#include <glm/mat4x4.hpp> // for mat4x4
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

// defines a `mat3x3` type to hold basic 2D transformations:
//  - translation
//  - scaling
//  - rotation
//
// Also provides operator overloading for:
//  - mat3x3 * mat3x3 = mat3x3
//  - mat3x3 * SDL_FPoint = SDL_FPoint
//  - mat3x3 * SDL_FRect = SDL_FRect

struct local_transform
{
    glm::vec3 position{0.0f}; // x, y, z
    glm::vec3 scale{1.0f};    // sx, sy, sz
    glm::vec3 rotation{0.0f}; // Euler angles in radians (pitch, yaw, roll)
};

struct parent
{
    entt::entity entity{entt::null};
};

namespace internal
{
    struct local_to_world
    {
        glm::mat4x4 mat;
    };

    struct children
    {
        std::unordered_set<entt::entity> entities;
    };
}

void parent_system(entt::registry &registry);
void local_to_world_system(entt::registry &registry);
glm::mat4 make_transform(const local_transform &t);
