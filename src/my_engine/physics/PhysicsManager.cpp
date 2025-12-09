#include "my_engine/physics/PhysicsManager.h"

#include <algorithm>

#include "my_engine/physics/RigidBody.h"

namespace
{
bool Contains(const std::vector<physics::Body*>& bodies, const physics::Body* body)
{
    return std::find(bodies.begin(), bodies.end(), body) != bodies.end();
}

bool Contains(const std::vector<physics::Joint*>& joints, const physics::Joint* joint)
{
    return std::find(joints.begin(), joints.end(), joint) != joints.end();
}
} // namespace

PhysicsManager* gPhysicsManager = nullptr;

PhysicsManager::PhysicsManager(Vec2 gravity, int iterations)
    : world(gravity, iterations)
{
}

void PhysicsManager::RegisterBody(RigidBody* rigidBody)
{
    if (rigidBody == nullptr)
        return;

    physics::Body* body = rigidBody->GetBody();
    if (body == nullptr || Contains(bodies, body))
        return;

    if (std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody) == rigidBodies.end())
        rigidBodies.push_back(rigidBody);
    bodies.push_back(body);
    world.Add(body);
}

void PhysicsManager::UnregisterBody(RigidBody* rigidBody)
{
    if (rigidBody == nullptr)
        return;

    physics::Body* body = rigidBody->GetBody();
    rigidBodies.erase(std::remove(rigidBodies.begin(), rigidBodies.end(), rigidBody), rigidBodies.end());

    if (body == nullptr)
        return;

    bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
    RemoveBodyFromWorld(body);
}

void PhysicsManager::RegisterJoint(physics::Joint* joint)
{
    if (joint == nullptr || Contains(joints, joint))
        return;

    joints.push_back(joint);
    world.Add(joint);
}

void PhysicsManager::UnregisterJoint(physics::Joint* joint)
{
    if (joint == nullptr)
        return;

    joints.erase(std::remove(joints.begin(), joints.end(), joint), joints.end());
    RemoveJointFromWorld(joint);
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

    for (physics::Joint* joint : joints)
    {
        if (joint == nullptr)
            continue;

        world.Add(joint);
    }
}

void PhysicsManager::ResetBodies()
{
    for (RigidBody* rigidBody : rigidBodies)
    {
        if (rigidBody == nullptr)
            continue;

        rigidBody->Reset();
    }

    RebuildWorld();
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

void PhysicsManager::RemoveJointFromWorld(physics::Joint* joint)
{
    world.joints.erase(std::remove(world.joints.begin(), world.joints.end(), joint), world.joints.end());
}
