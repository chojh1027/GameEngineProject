#include "my_engine/physics/JointComponent.h"

#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/physics/RigidBody.h"

JointComponent::JointComponent(GameObject* owner,
                               RigidBody* firstBody,
                               RigidBody* secondBody,
                               const Vec2& anchor)
    : Component(owner)
    , bodyA(firstBody)
    , bodyB(secondBody)
    , anchorPoint(anchor)
    , anchorIsExplicit(anchor.x != 0.0f || anchor.y != 0.0f)
{
}

void JointComponent::Init()
{
    ConfigureJoint();
}

void JointComponent::Start()
{
    ConfigureJoint();
    RegisterJoint();
}

void JointComponent::Destroy()
{
    UnregisterJoint();
}

void JointComponent::SetBodies(RigidBody* firstBody, RigidBody* secondBody)
{
    if (firstBody == bodyA && secondBody == bodyB)
        return;

    bodyA = firstBody;
    bodyB = secondBody;

    ConfigureJoint();
}

void JointComponent::SetAnchor(const Vec2& newAnchor)
{
    anchorPoint = newAnchor;
    anchorIsExplicit = true;
    ConfigureJoint();
}

void JointComponent::ConfigureJoint()
{
    if (bodyA == nullptr || bodyB == nullptr)
        return;

    physics::Body* firstPhysicsBody = bodyA->GetBody();
    physics::Body* secondPhysicsBody = bodyB->GetBody();
    if (firstPhysicsBody == nullptr || secondPhysicsBody == nullptr)
        return;

    Vec2 anchor = anchorPoint;
    if (!anchorIsExplicit)
    {
        anchor = (firstPhysicsBody->position + secondPhysicsBody->position) * 0.5f;
        anchorPoint = anchor;
    }

    joint.Set(firstPhysicsBody, secondPhysicsBody, anchor);
}

void JointComponent::RegisterJoint()
{
    if (isRegistered || gPhysicsManager == nullptr)
        return;

    if (bodyA == nullptr || bodyB == nullptr)
        return;

    gPhysicsManager->RegisterJoint(&joint);
    isRegistered = true;
}

void JointComponent::UnregisterJoint()
{
    if (!isRegistered || gPhysicsManager == nullptr)
        return;

    gPhysicsManager->UnregisterJoint(&joint);
    isRegistered = false;
}
