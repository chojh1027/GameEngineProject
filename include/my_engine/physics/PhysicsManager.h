#pragma once
#ifndef MY_ENGINE_PHYSICS_MANAGER_H
#define MY_ENGINE_PHYSICS_MANAGER_H

#include <vector>

#include "box2d-lite/MathUtils.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/Joint.h"
#include "my_engine/physics/World.h"

class PhysicsManager
{
public:
    PhysicsManager(Vec2 gravity, int iterations);

    void RegisterBody(class RigidBody* rigidBody);
    void UnregisterBody(class RigidBody* rigidBody);

    void RegisterJoint(physics::Joint* joint);
    void UnregisterJoint(physics::Joint* joint);

    void Step(float deltaTime);
    void RebuildWorld();
    void ResetBodies();

    const std::vector<physics::Body*>& GetBodies() const { return bodies; }
    const std::vector<physics::Joint*>& GetJoints() const { return joints; }
    const std::vector<class RigidBody*>& GetRigidBodies() const { return rigidBodies; }
    const physics::World& GetWorld() const { return world; }

private:
    void RemoveBodyFromWorld(physics::Body* body);
    void RemoveJointFromWorld(physics::Joint* joint);

    physics::World world;
    std::vector<physics::Body*> bodies;
    std::vector<class RigidBody*> rigidBodies;
    std::vector<physics::Joint*> joints;
};

extern PhysicsManager* gPhysicsManager;

#endif // MY_ENGINE_PHYSICS_MANAGER_H
