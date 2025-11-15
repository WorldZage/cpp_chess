#include "engine/engine.hpp"
#include "engine/game_objects.hpp"
#include "engine/camera.hpp"
#include "game/game.hpp"

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    AppState *state = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!state)
    {
        return SDL_APP_FAILURE;
    }

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_RESIZABLE, &(state->window), &(state->renderer)))
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        SDL_DestroyWindow(state->window);
        return SDL_APP_FAILURE;
    }

    state->is_running = true;
    state->last_frame_end = SDL_GetTicksNS();
    state->scene = initial_scene();
    *appstate = state;

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (appstate == nullptr)
    {
        return SDL_APP_FAILURE;
    }
    AppState *state = (AppState *)appstate;

    if (event->type == SDL_EVENT_QUIT || state->scene == nullptr)
    {
        return SDL_APP_SUCCESS;
    }
    else
    {
        state->scene->handle_event(event);
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (appstate == nullptr)
    {
        return SDL_APP_FAILURE;
    }
    AppState *state = (AppState *)appstate;

    Uint64 frame_begin = SDL_GetTicksNS();
    float delta_time = (frame_begin - state->last_frame_end) / SDL_NS_PER_SECOND;

    state->scene->update();
    state->scene->render(state->renderer);

    /*
     */
    state->last_frame_end = SDL_GetTicksNS();
    Uint64 elapsed = state->last_frame_end - frame_begin;
    Uint64 target = SDL_NS_PER_SECOND / FPS;

    if (elapsed < target)
    {
        SDL_DelayNS(target - elapsed);
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (appstate == nullptr)
    {
        SDL_Quit();
    }
    AppState *state = (AppState *)appstate;

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}
