#include "my_engine/Rigidbody.h"

#include <cfloat>

#include "my_engine/GameObject.h"
#include "my_engine/Transform.h"

namespace
{
	constexpr float k_minDensity = 0.0001f;
	constexpr float k_minDimension = 0.0001f;
}

Rigidbody::Rigidbody(GameObject* gameObject)
        : Component(gameObject)
{
        body.Set(Vec2(1.0f, 1.0f), 1.0f);
        SyncBodyFromTransform();
}

void Rigidbody::Init()
{
	InitializeBody();
}

void Rigidbody::Update(float deltaTime)
{
	if (bodyType == BodyType::Static || deltaTime <= 0.0f)
		return;

	Vec2 totalForce = accumulatedForces;
	if (useGravity)
	{
		Vec2 gravity = GetGlobalGravity();
		totalForce += (gravityScale * body.mass) * gravity;
	}

	Vec2 acceleration(0.0f, 0.0f);
	if (body.mass < FLT_MAX)
		acceleration = body.invMass * totalForce;

	body.velocity += deltaTime * acceleration;
	body.position += deltaTime * body.velocity;

	float totalTorque = accumulatedTorque;
	float angularAcceleration = 0.0f;
	if (body.I < FLT_MAX && body.I > 0.0f)
		angularAcceleration = totalTorque * body.invI;

	body.angularVelocity += deltaTime * angularAcceleration;
	body.rotation += deltaTime * body.angularVelocity;

	accumulatedForces.Set(0.0f, 0.0f);
	accumulatedTorque = 0.0f;

	SyncTransformFromBody();
}

void Rigidbody::SetBodyType(BodyType type)
{
	bodyType = type;
	InitializeBody();
}

void Rigidbody::SetBoxShape(const Vec2& size)
{
	colliderType = ColliderType::Box;
	boxSize = Vec2(Max(size.x, k_minDimension), Max(size.y, k_minDimension));
	InitializeBody();
}

void Rigidbody::SetCircleShape(float radius)
{
	colliderType = ColliderType::Circle;
	circleRadius = Max(radius, k_minDimension);
	InitializeBody();
}

void Rigidbody::SetDensity(float newDensity)
{
	density = newDensity < k_minDensity ? k_minDensity : newDensity;
	InitializeBody();
}

void Rigidbody::SetGravityScale(float scale)
{
	gravityScale = scale;
}

void Rigidbody::UseGravity(bool value)
{
	useGravity = value;
}

void Rigidbody::ApplyForce(const Vec2& force)
{
	if (bodyType == BodyType::Static)
		return;

	accumulatedForces += force;
}

void Rigidbody::ApplyImpulse(const Vec2& impulse)
{
	if (bodyType == BodyType::Static || body.mass >= FLT_MAX)
		return;

	body.velocity += body.invMass * impulse;
}

void Rigidbody::ApplyTorque(float torque)
{
	if (bodyType == BodyType::Static)
		return;

	accumulatedTorque += torque;
}

void Rigidbody::SetVelocity(const Vec2& velocity)
{
	if (bodyType == BodyType::Static)
		return;

	body.velocity = velocity;
}

void Rigidbody::SetAngularVelocity(float angularVelocity)
{
	if (bodyType == BodyType::Static)
		return;

	body.angularVelocity = angularVelocity;
}

void Rigidbody::InitializeBody()
{
	SyncBodyFromTransform();

        const Vec2 width = GetColliderWidth();
        float mass = bodyType == BodyType::Static ? FLT_MAX : ComputeMass();

        Body::ShapeType shapeType = colliderType == ColliderType::Circle ? Body::ShapeType::Circle : Body::ShapeType::Box;
        body.Set(width, mass, shapeType);

        accumulatedForces.Set(0.0f, 0.0f);
        accumulatedTorque = 0.0f;

	if (bodyType == BodyType::Static)
	{
		body.velocity.Set(0.0f, 0.0f);
		body.angularVelocity = 0.0f;
	}
        SyncBodyFromTransform();
}

Vec2 Rigidbody::GetColliderWidth() const
{
	if (colliderType == ColliderType::Circle)
	{
		const float diameter = circleRadius * 2.0f;
		return Vec2(diameter, diameter);
	}

	return boxSize;
}

float Rigidbody::ComputeMass() const
{
	if (colliderType == ColliderType::Circle)
	{
		return density * k_pi * circleRadius * circleRadius;
	}

	return density * boxSize.x * boxSize.y;
}

void Rigidbody::SyncTransformFromBody()
{
	if (gameObject == nullptr || gameObject->transform == nullptr)
		return;

	gameObject->transform->SetPosition(body.position);
	gameObject->transform->SetRotation(body.rotation);
}

void Rigidbody::SyncBodyFromTransform()
{
	if (gameObject == nullptr || gameObject->transform == nullptr)
		return;

	body.position = gameObject->transform->GetPosition();
	body.rotation = gameObject->transform->GetRotation();
}

Vec2 Rigidbody::GetGlobalGravity()
{
	return Vec2(0.0f, -9.81f);
}
