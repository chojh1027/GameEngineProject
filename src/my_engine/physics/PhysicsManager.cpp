#include "my_engine/physics/PhysicsManager.h"

#include <algorithm>

namespace
{
bool Contains(const std::vector<physics::Body*>& bodies, const physics::Body* body)
{
    return std::find(bodies.begin(), bodies.end(), body) != bodies.end();
}
} // namespace

PhysicsManager::PhysicsManager(Vec2 gravity, int iterations)
    : world(gravity, iterations)
{
}

void PhysicsManager::RegisterBody(physics::Body* body)
{
    if (body == nullptr || Contains(bodies, body))
        return;

    bodies.push_back(body);
    world.Add(body);
}

void PhysicsManager::UnregisterBody(physics::Body* body)
{
    if (body == nullptr)
        return;

    bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
    RemoveBodyFromWorld(body);
}

void PhysicsManager::Step(float deltaTime)
{
    world.Step(deltaTime);
}

void PhysicsManager::RebuildWorld()
{
    world.Clear();

    for (physics::Body* body : bodies)
    {
        if (body == nullptr)
            continue;

        world.Add(body);
    }
}

void PhysicsManager::RemoveBodyFromWorld(physics::Body* body)
{
    world.bodies.erase(std::remove(world.bodies.begin(), world.bodies.end(), body), world.bodies.end());

    for (auto iter = world.arbiters.begin(); iter != world.arbiters.end(); )
    {
        const physics::Arbiter& arbiter = iter->second;
        if (arbiter.body1 == body || arbiter.body2 == body)
        {
            iter = world.arbiters.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}
