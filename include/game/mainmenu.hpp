#pragma once

#include "engine/scene.hpp"

// MainMenuScene.hpp
#include <iostream>

class MainMenuScene : public Scene
{
public:
    MainMenuScene()
    {
        init();
    }

    bool init() override
    {

        SDL_Log("Main menu initialized");
        // Create a camera entity
        auto e_camera = m_registry.create();
        auto &c_camera = m_registry.emplace<camera>(e_camera);
        c_camera.z_index = 0;
        c_camera.view = SDL_FRect{0, 0, 800, 600};     // world view area
        c_camera.viewport = SDL_FRect{0, 0, 800, 600}; // screen viewport
        auto &c_cam_transform = m_registry.emplace<local_transform>(e_camera);

        // auto &c_cam_draw_target = m_registry.emplace<internal::draw_target>(e_camera);

        // Create drawable entity
        auto e_button = m_registry.create();

        auto &c_drawable = m_registry.emplace<drawable>(e_button);
        c_drawable.depth = 0;
        c_drawable.bounding_box = SDL_FRect{0.0f, 0.0f, 200.0f, 80.0f};

        auto &c_transform = m_registry.emplace<local_transform>(e_button);

        // Define draw lambda (using your system's signature)
        c_drawable.draw = [](SDL_Renderer *renderer, glm::mat4 transform, float dt)
        {
            bool demo = false;
            if (demo)
            {
                const char *message = "Hell World!";
                int w = 0, h = 0;
                float x, y;
                const float scale = 4.0f;

                /* Center the message and scale it up */
                SDL_GetRenderOutputSize(renderer, &w, &h);
                SDL_SetRenderScale(renderer, scale, scale);
                x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
                y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

                /* Draw the message */
                /*
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                */
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                SDL_RenderDebugText(renderer, x, y, message);
                SDL_RenderPresent(renderer);
            }

            // Transform the rect's corners into world space
            SDL_FRect rect{0.0f, 0.0f, 200.0f, 80.0f};
            auto p0 = to_sdl_point(transform * to_vec4({rect.x, rect.y}));
            auto p1 = to_sdl_point(transform * to_vec4({rect.x + rect.w, rect.y + rect.h}));

            SDL_FRect screen_rect{
                p0.x,
                p0.y,
                p1.x - p0.x,
                p1.y - p0.y,
            };

            SDL_SetRenderDrawColor(renderer, 255, 128, 64, 255);
            SDL_RenderFillRect(renderer, &screen_rect);
        };
        return true;
    }

    void update() override
    {
        registry_updates();
    }

    void render(SDL_Renderer *renderer) override
    {
        internal::render_system(m_registry, renderer, 0.1f);
        // draw buttons, logo, etc.
    }

    void handle_event(SDL_Event *event) override
    {
        if (event->type == SDL_EVENT_KEY_DOWN)
        {
            std::cout << "Main menu key press\n";
            SDL_Log("Key: %s", SDL_GetKeyName(event->key.key));
        }
    }

    void clean()
    {
        std::cout << "Main menu cleaned up\n";
    }
};
