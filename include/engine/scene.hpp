#pragma once

/**
 * @brief The Scene class holds all assets to manage and render the visual
 * and non-visual entities within the displayed scene on screen
 */
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "engine/game_objects.hpp"
#include "engine/camera.hpp"

class Scene
{
public:
    // constructor and destructor implemented inline
    Scene()
    {
        internal::camera_setup_system(m_registry);
    };
    ~Scene() { clean(); };

    void registry_updates()
    {
        parent_system(m_registry);
        local_to_world_system(m_registry);
        bounding_box_system(m_registry);
    };

    // initialize assets and entities
    virtual bool init() { return true; }

    // delegates to the game loop
    virtual void handle_event(SDL_Event *event) = 0;
    virtual void update() = 0;
    virtual void render(SDL_Renderer *renderer) = 0;

    // clear registry and release resources
    virtual void clean() {};

protected:
    // EnTT registry to register and manage all entities
    entt::registry m_registry;
};