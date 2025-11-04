#pragma once
#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "box2d-lite/Body.h"
#include "box2d-lite/MathUtils.h"
#include "my_engine/Component.h"

class Rigidbody : public Component
{
public:
	enum class BodyType
	{
		Static,
		Dynamic
	};

	enum class ColliderType
	{
		Box,
		Circle
	};

	explicit Rigidbody(GameObject* gameObject);

	void Init() override;
	void Update(float deltaTime) override;

	void SetBodyType(BodyType type);
	BodyType GetBodyType() const { return bodyType; }

	void SetBoxShape(const Vec2& size);
	void SetCircleShape(float radius);

	ColliderType GetColliderType() const { return colliderType; }
	Vec2 GetBoxSize() const { return boxSize; }
	float GetCircleRadius() const { return circleRadius; }

	void SetDensity(float newDensity);
	float GetDensity() const { return density; }

	void SetGravityScale(float scale);
	float GetGravityScale() const { return gravityScale; }

	void UseGravity(bool value);
	bool IsUsingGravity() const { return useGravity; }

	void ApplyForce(const Vec2& force);
	void ApplyImpulse(const Vec2& impulse);
	void ApplyTorque(float torque);

	void SetVelocity(const Vec2& velocity);
	Vec2 GetVelocity() const { return body.velocity; }

	void SetAngularVelocity(float angularVelocity);
	float GetAngularVelocity() const { return body.angularVelocity; }

	const Body& GetBody() const { return body; }

private:
	void InitializeBody();
	Vec2 GetColliderWidth() const;
	float ComputeMass() const;
	float ComputeInertia(float mass) const;
	void SyncTransformFromBody();
	void SyncBodyFromTransform();

	static Vec2 GetGlobalGravity();

	BodyType bodyType = BodyType::Dynamic;
	ColliderType colliderType = ColliderType::Box;

	Vec2 boxSize = Vec2(1.0f, 1.0f);
	float circleRadius = 0.5f;
	float density = 1.0f;
	float gravityScale = 1.0f;
	bool useGravity = true;

	Vec2 accumulatedForces = Vec2(0.0f, 0.0f);
	float accumulatedTorque = 0.0f;

	Body body;
};

#endif // RIGIDBODY_H
