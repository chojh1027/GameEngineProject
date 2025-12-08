#include "my_engine/physics/RigidBody.h"

#include "my_engine/GameObject.h"
#include "my_engine/Transform.h"
#include "my_engine/physics/PhysicsManager.h"

RigidBody::RigidBody(GameObject* owner,
                     PhysicsManager& manager,
                     const Vec2& size,
                     float mass,
                     physics::Body::ShapeType shape,
                     const Vec2& initialPosition,
                     float initialRotation)
    : Component(owner)
    , physicsManager(manager)
    , size(size)
    , mass(mass)
    , shape(shape)
    , initialPosition(initialPosition)
    , initialRotation(initialRotation)
{
}

void RigidBody::Init()
{
    body.Set(size, mass, shape);
    body.position = initialPosition;
    body.rotation = initialRotation;
}

void RigidBody::Start()
{
    RegisterBody();
    SyncTransform();
}

void RigidBody::FixedUpdate(float fixedDeltaTime)
{
    (void)fixedDeltaTime;
    SyncTransform();
}

void RigidBody::Destroy()
{
    UnregisterBody();
}

void RigidBody::Reset()
{
    UnregisterBody();
    body.Set(size, mass, shape);
    body.position = initialPosition;
    body.rotation = initialRotation;
    RegisterBody();
    SyncTransform();
}

void RigidBody::SetInitialTransform(const Vec2& position, float rotation)
{
    initialPosition = position;
    initialRotation = rotation;
    body.position = position;
    body.rotation = rotation;
    SyncTransform();
}

void RigidBody::RegisterBody()
{
    if (isRegistered)
        return;

    physicsManager.RegisterBody(&body);
    isRegistered = true;
}

void RigidBody::UnregisterBody()
{
    if (!isRegistered)
        return;

    physicsManager.UnregisterBody(&body);
    isRegistered = false;
}

void RigidBody::SyncTransform()
{
    if (gameObject == nullptr || gameObject->transform == nullptr)
        return;

    gameObject->transform->SetPosition(body.position);
    gameObject->transform->SetRotation(body.rotation);
}
