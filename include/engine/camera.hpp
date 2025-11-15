#pragma once

#include <functional>
#include <queue>
#include <SDL3/SDL.h>
#include "engine/game_objects.hpp"

struct drawable
{
    Sint64 depth;
    SDL_FRect bounding_box;
    std::function<void(SDL_Renderer *, glm::mat4, float)> draw;

    struct compare
    {
        bool operator()(const drawable &lhs, const drawable &rhs) const;
    };
};

struct camera
{
    Sint64 z_index;
    SDL_FRect view;
    SDL_FRect viewport;

    struct compare
    {
        bool operator()(const camera &lhs, const camera &rhs) const;
    };
};

void bounding_box_system(entt::registry &registry);

namespace internal
{

    struct bounding_box
    {
        SDL_FRect rect;
    };

    struct draw_call
    {
        drawable m_drawable;
        glm::mat4x4 world_transform;

        struct compare
        {
            bool operator()(const draw_call &lhs, const draw_call &rhs) const;
        };
    };

    struct draw_target
    {
        using draw_queue_type = std::priority_queue<
            draw_call,
            std::vector<draw_call>,
            draw_call::compare>;

        draw_queue_type queue;
        SDL_Texture *target;
    };

    static void construct_draw_target(entt::registry &registry, entt::entity entity);

    static void destroy_draw_target(entt::registry &registry, entt::entity entity);
    void camera_setup_system(entt::registry &registry);
    static SDL_Texture *prepare_draw_target_texture(
        SDL_Renderer *renderer,
        SDL_Texture *target,
        SDL_FRect view);

    void render_system(
        entt::registry &registry,
        SDL_Renderer *renderer,
        float delta_time);
};

SDL_FRect transform_rect(glm::mat4x4 transform, SDL_FRect rect);

// Convert SDL_FPoint to glm::vec4 (homogeneous coordinates)
inline glm::vec4 to_vec4(const SDL_FPoint &p, float z = 0.0f, float w = 1.0f)
{
    return glm::vec4(p.x, p.y, z, w);
}

// Convert glm::vec4 to SDL_FPoint
inline SDL_FPoint to_sdl_point(const glm::vec4 &v)
{
    return SDL_FPoint{v.x, v.y};
}
