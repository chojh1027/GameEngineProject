#pragma once
#ifndef MY_ENGINE_PHYSICS_RIGID_BODY_H
#define MY_ENGINE_PHYSICS_RIGID_BODY_H

#include "my_engine/Component.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/PhysicsManager.h"

class RigidBody : public Component
{
public:
    RigidBody(GameObject* owner,
              const Vec2& size,
              float mass,
              physics::Body::ShapeType shape = physics::Body::ShapeType::Box,
              const Vec2& initialPosition = Vec2(0.0f, 0.0f),
              float initialRotation = 0.0f);

    // ~RigidBody() override = default;

    void Init() override;
    void Start() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Destroy() override;

    void Reset();

    physics::Body* GetBody() { return &body; }
    const physics::Body* GetBody() const { return &body; }

    void SetInitialTransform(const Vec2& position, float rotation);

	void SetVelocity(const Vec2& velocity);
	void AddForce(const Vec2& force);

    void SetInertia(float inertia)
    {
		body.I = inertia;
	}

    void SetFriction(float friction)
    {
        body.friction = friction;
    }

private:
    void InitializeBodyState();
    void RegisterBody();
    void UnregisterBody();
    void SyncTransform();

    physics::Body body;
    Vec2 size;
    float mass;
    physics::Body::ShapeType shape;
    Vec2 initialPosition;
    float initialRotation;
    bool isRegistered = false;
};

#endif // MY_ENGINE_PHYSICS_RIGID_BODY_H
