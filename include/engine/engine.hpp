#pragma once

#define FPS 60

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <entt/entt.hpp>

#include "scene.hpp"

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;
    Scene *scene;

    Uint64 last_frame_end;
} AppState;
