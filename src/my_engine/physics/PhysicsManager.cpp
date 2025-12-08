#include "my_engine/physics/PhysicsManager.h"

#include <cfloat>

namespace
{
constexpr int kCircleStackCount = 5;
constexpr float kGroundWidth = 100.0f;
constexpr float kGroundHeight = 20.0f;
constexpr float kGroundYOffset = -0.5f;
constexpr float kCircleRadius = 1.0f;
constexpr float kCircleMass = 10.0f;
constexpr float kCircleStartY = 8.0f;
constexpr float kCircleStartX = -6.0f;
constexpr float kCircleSpacing = 3.0f;
} // namespace

PhysicsManager::PhysicsManager(Vec2 gravity, int iterations)
        : world(gravity, iterations)
{
}

void PhysicsManager::InitializeCircleStage()
{
        world.Clear();
        bodies.clear();

        physics::Body* ground = CreateBody(Vec2(kGroundWidth, kGroundHeight), FLT_MAX);
        if (ground != nullptr)
        {
                ground->position.Set(0.0f, kGroundYOffset * ground->width.y);
        }

        for (int i = 0; i < kCircleStackCount; ++i)
        {
                physics::Body* circle = CreateBody(Vec2(kCircleRadius, kCircleRadius), kCircleMass, physics::Body::ShapeType::Circle);
                if (circle == nullptr)
                        continue;

                circle->position.Set(kCircleStartX + kCircleSpacing * i, kCircleStartY);
        }
}

void PhysicsManager::Step(float deltaTime)
{
        world.Step(deltaTime);
}

physics::Body* PhysicsManager::CreateBody(const Vec2& size, float mass, physics::Body::ShapeType shape)
{
        std::unique_ptr<physics::Body> body = std::make_unique<physics::Body>();
        if (body == nullptr)
                return nullptr;

        body->Set(size, mass, shape);
        physics::Body* bodyPtr = body.get();
        bodies.push_back(std::move(body));
        world.Add(bodyPtr);
        return bodyPtr;
}
