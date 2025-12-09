#pragma once
#ifndef MY_ENGINE_PHYSICS_JOINT_COMPONENT_H
#define MY_ENGINE_PHYSICS_JOINT_COMPONENT_H

#include "box2d-lite/MathUtils.h"
#include "my_engine/Component.h"
#include "my_engine/physics/Joint.h"

class RigidBody;

class JointComponent : public Component
{
public:
    JointComponent(GameObject* owner,
                   RigidBody* firstBody,
                   RigidBody* secondBody,
                   const Vec2& anchor = Vec2(0.0f, 0.0f));

    void Init() override;
    void Start() override;
    void Destroy() override;

    void SetBodies(RigidBody* firstBody, RigidBody* secondBody);
    void SetAnchor(const Vec2& newAnchor);

    physics::Joint* GetJoint() { return &joint; }
    const physics::Joint* GetJoint() const { return &joint; }

private:
    void ConfigureJoint();
    void RegisterJoint();
    void UnregisterJoint();

    physics::Joint joint;
    RigidBody* bodyA = nullptr;
    RigidBody* bodyB = nullptr;
    Vec2 anchorPoint;
    bool anchorIsExplicit = false;
    bool isRegistered = false;
};

#endif // MY_ENGINE_PHYSICS_JOINT_COMPONENT_H
