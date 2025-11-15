#include "engine/game_objects.hpp"

void parent_system(entt::registry &registry)
{
    auto child_pov = registry.view<parent>();

    auto parent_pov = registry.view<internal::children>();
    // add to `children` every entity with a `parent`
    for (auto entity : child_pov)
    {
        auto &c_parent = registry.get<parent>(entity);

        if (registry.valid(c_parent.entity))
        {
            auto &c_children = registry.get_or_emplace<internal::children>(c_parent.entity);
            c_children.entities.insert(entity); // insert(entity);
        }
    }

    // remove from `children`:
    //  - every invalid entities
    //  - entities with the a different `parent`
    //  - entities without a `parent`
    for (auto entity : parent_pov)
    {
        auto &c_children = parent_pov.get<internal::children>(entity);

        auto to_remove = std::vector<entt::entity>{};
        to_remove.reserve(c_children.entities.size());

        for (auto child : c_children.entities)
        {
            if (!registry.valid(child))
            {
                // the entity was destroyed
                to_remove.push_back(child);
            }
            else if (child_pov.contains(child))
            {
                auto &c_parent = registry.get<parent>(child);

                if (c_parent.entity != entity)
                {
                    // the entity was assigned to another parent
                    to_remove.push_back(child);
                }
            }
            else
            {
                // the entity does not have a parent anymore
                to_remove.push_back(child);
            }
        }

        for (auto child : to_remove)
        {
            c_children.entities.erase(child);
        }
    }
    /*
     */
}

void local_to_world_system(entt::registry &registry)
{
    auto view = registry.view<local_transform>();
    auto visited = entt::sparse_set();
    //    std::unordered_set<entt::entity>{};

    // recursive function to traverse the hierarchy
    // `self` is the function itself, for recursion,
    // see: https://en.wikipedia.org/wiki/Fixed-point_combinator
    auto compute_world_matrix = [&](auto &self, entt::entity e) -> glm::mat4x4
    {
        if (visited.contains(e))
        {
            // we already visited this entity, let's return its world matrix
            auto &c_local_to_world = registry.get<internal::local_to_world>(e);
            return c_local_to_world.mat;
        }

        auto local_matrix = glm::mat4x4(1.0f);

        // it is possible that the developer referenced a parent entity that
        // has no local transform.

        if (view.contains(e))
        {
            // but if it does, let's compute the local matrix
            const auto &c_local_transform = view.get<local_transform>(e);

            local_matrix = make_transform(c_local_transform);
        }

        auto world_matrix = local_matrix;

        // if the entity has a parent and this parent is a valid entity,
        // let's recurse
        auto *c_parent = registry.try_get<parent>(e);

        if (c_parent != nullptr && registry.valid(c_parent->entity))
        {
            auto parent_world_matrix = self(self, c_parent->entity);
            world_matrix = parent_world_matrix * world_matrix;
        }

        // finally update or insert the final component
        registry.emplace_or_replace<internal::local_to_world>(e, world_matrix);
        visited.push(e);

        return world_matrix;
    };

    for (auto entity : view)
    {
        compute_world_matrix(compute_world_matrix, entity);
    }
}

glm::mat4 make_transform(const local_transform &t)
{
    glm::mat4 mat(1.0f);

    // Translate
    mat = glm::translate(mat, t.position);

    // Rotate (Euler angles)
    mat = glm::rotate(mat, t.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
    mat = glm::rotate(mat, t.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
    mat = glm::rotate(mat, t.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll

    // Scale
    mat = glm::scale(mat, t.scale);

    return mat;
}
