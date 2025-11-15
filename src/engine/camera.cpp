#include "engine/camera.hpp"

/* treat your 2D coordinates as if they’re in the XY-plane at z=0:
[x,y,0,1]
*/

bool drawable::compare::operator()(const drawable &lhs, const drawable &rhs) const
{
    return lhs.depth < rhs.depth;
}

bool camera::compare::operator()(const camera &lhs, const camera &rhs) const
{
    return lhs.z_index > rhs.z_index;
}

bool internal::draw_call::compare::operator()(const draw_call &lhs, const draw_call &rhs) const
{
    return drawable::compare{}(lhs.m_drawable, rhs.m_drawable);
}

SDL_FRect transform_rect(glm::mat4x4 transform, SDL_FRect rect)
{
    // Define the 4 corners of the rectangle in local space
    glm::vec4 corners[4] = {
        {rect.x, rect.y, 0.0f, 1.0f},                  // top-left
        {rect.x + rect.w, rect.y, 0.0f, 1.0f},         // top-right
        {rect.x, rect.y + rect.h, 0.0f, 1.0f},         // bottom-left
        {rect.x + rect.w, rect.y + rect.h, 0.0f, 1.0f} // bottom-right
    };

    // Transform each corner
    for (auto &c : corners)
        c = transform * c;

    // Find min/max in transformed space
    float min_x = std::min({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
    float max_x = std::max({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
    float min_y = std::min({corners[0].y, corners[1].y, corners[2].y, corners[3].y});
    float max_y = std::max({corners[0].y, corners[1].y, corners[2].y, corners[3].y});

    return SDL_FRect{
        min_x,
        min_y,
        max_x - min_x,
        max_y - min_y};
}

void bounding_box_system(entt::registry &registry)
{
    auto view = registry.view<drawable>();
    auto visited = entt::sparse_set();
    // auto visited = std::unordered_set<entt::entity>{};
    visited.reserve(view.size());

    auto compute_bbox = [&](auto &self, entt::entity e) -> SDL_FRect
    {
        if (visited.contains(e))
        {
            auto &c_bbox = registry.get<internal::bounding_box>(e);
            return c_bbox.rect;
        }

        auto bbox = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};

        // child entities may have no drawables, but their children might have.
        // let's check if the component exists and assume a default empty rect
        // for the bounding box

        auto *c_drawable = registry.try_get<drawable>(e);
        auto *c_transform = registry.try_get<internal::local_to_world>(e);
        auto *c_children = registry.try_get<internal::children>(e);

        if (c_drawable != nullptr && c_transform != nullptr)
        {
            // we have a drawable, let's compute the actual local bounding box
            bbox = transform_rect(c_transform->mat, c_drawable->bounding_box);
        }

        if (c_children != nullptr)
        {
            // we have children, let's get the union of their bounding box
            for (auto child : c_children->entities)
            {
                // get child bounding box (recursively)
                auto child_bbox = self(self, child);

                // bbox U child_bbox --> bbox
                SDL_GetRectUnionFloat(&bbox, &child_bbox, &bbox);
            }
        }

        // add the component to the entity
        registry.emplace_or_replace<internal::bounding_box>(e, bbox);
        visited.push(e);

        return bbox;
    };

    for (auto entity : view)
    {
        compute_bbox(compute_bbox, entity);
    }
}

void internal::construct_draw_target(entt::registry &registry, entt::entity entity)
{
    registry.emplace<internal::draw_target>(entity);
}

void internal::destroy_draw_target(entt::registry &registry, entt::entity entity)
{
    registry.remove<internal::draw_target>(entity);
}
void internal::camera_setup_system(entt::registry &registry)
{
    registry.on_construct<camera>().connect<&construct_draw_target>();
    registry.on_destroy<camera>().connect<&destroy_draw_target>();
}

SDL_Texture *internal::prepare_draw_target_texture(
    SDL_Renderer *renderer,
    SDL_Texture *target,
    SDL_FRect view)
{
    if (target != nullptr)
    {
        float w, h;
        SDL_GetTextureSize(target, &w, &h);

        if (w != view.w || h != view.h)
        {
            SDL_DestroyTexture(target);
            target = nullptr;
        }
    }

    if (target == nullptr)
    {
        target = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            view.w, view.h);
    }

    return target;
}

void internal::render_system(
    entt::registry &registry,
    SDL_Renderer *renderer,
    float delta_time)
{
    registry.sort<camera>(camera::compare{});
    registry.sort<drawable>(drawable::compare{});

    auto camera_entities = registry.view<
        camera,
        internal::local_to_world,
        internal::draw_target>();
    // iterate over entities using the `camera` component ordering
    camera_entities.use<camera>();

    auto drawable_entities = registry.view<
        drawable,
        internal::local_to_world,
        internal::bounding_box>();
    // iterate over entities using the `drawable` component ordering
    drawable_entities.use<drawable>();

    for (auto e_tuple_camera : camera_entities.each())
    {
        entt::entity e_camera = std::get<0>(e_tuple_camera);
        auto &c_camera = camera_entities.get<camera>(e_camera);
        auto &c_camera_transform = camera_entities.get<internal::local_to_world>(e_camera);
        auto &c_draw_target = camera_entities.get<internal::draw_target>(e_camera);

        c_draw_target.target = prepare_draw_target_texture(
            renderer,
            c_draw_target.target,
            c_camera.view);
        if (c_draw_target.target == nullptr)
        {
            continue;
        }

        SDL_SetRenderTarget(renderer, c_draw_target.target);

        // clear target
        auto bak = SDL_Color{};
        SDL_GetRenderDrawColor(renderer, &bak.r, &bak.g, &bak.b, &bak.a);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, bak.r, bak.g, bak.b, bak.a);

        auto view_local_pos = SDL_FPoint{c_camera.view.x, c_camera.view.y};
        auto view_world_pos4 = c_camera_transform.mat * to_vec4(view_local_pos);
        auto view_world_pos = to_sdl_point(view_world_pos4);

        auto world_view = SDL_FRect{
            .x = view_world_pos.x - c_camera.view.w / 2.0f,
            .y = view_world_pos.y - c_camera.view.h / 2.0f,
            .w = c_camera.view.w,
            .h = c_camera.view.h,
        };

        for (auto e_tuple_drawable : drawable_entities.each())
        {
            entt::entity e_drawable = std::get<0>(e_tuple_drawable);
            auto &c_drawable = drawable_entities.get<drawable>(e_drawable);
            auto &c_drawable_transform = drawable_entities.get<internal::local_to_world>(e_drawable);
            auto &c_bounding_box = drawable_entities.get<internal::bounding_box>(e_drawable);

            if (SDL_HasRectIntersectionFloat(&world_view, &c_bounding_box.rect))
            {
                // entity's bounding box intersects with the camera view

                // compute world space to screen space transformation matrix

                auto M_offset = glm::translate(glm::mat4x4(1.0f), glm::vec3(-view_world_pos.x, -view_world_pos.y, 0.0f));
                glm::mat4 M_center = glm::translate(glm::mat4x4(1.0f), glm::vec3(c_camera.view.w / 2.0f, c_camera.view.h / 2.0f, 0.0f));

                auto M = (M_center * M_offset) * c_drawable_transform.mat;

                // push draw call
                c_draw_target.queue.emplace(internal::draw_call{
                    .m_drawable = c_drawable,
                    .world_transform = M,
                });
            }
        }

        // execute draw calls
        while (!c_draw_target.queue.empty())
        {
            auto draw_call = c_draw_target.queue.top();
            c_draw_target.queue.pop();

            draw_call.m_drawable.draw(renderer, draw_call.world_transform, delta_time);
        }

        // render camera texture to screen
        // NB: SDL will automatically stretch the texture to the viewport
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderTexture(renderer, c_draw_target.target, nullptr, &c_camera.viewport);
    }
    SDL_RenderPresent(renderer);
}