#pragma once
#ifndef MY_ENGINE_PHYSICS_MANAGER_H
#define MY_ENGINE_PHYSICS_MANAGER_H

#include <memory>
#include <vector>

#include "box2d-lite/MathUtils.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/World.h"

class PhysicsManager
{
public:
        PhysicsManager(Vec2 gravity, int iterations);

        void InitializeCircleStage();
        void Step(float deltaTime);

        const std::vector<std::unique_ptr<physics::Body>>& GetBodies() const { return bodies; }
        const physics::World& GetWorld() const { return world; }

private:
        physics::Body* CreateBody(const Vec2& size, float mass, physics::Body::ShapeType shape = physics::Body::ShapeType::Rect);

        physics::World world;
        std::vector<std::unique_ptr<physics::Body>> bodies;
};

#endif // MY_ENGINE_PHYSICS_MANAGER_H
