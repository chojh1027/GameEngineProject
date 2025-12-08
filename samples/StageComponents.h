#pragma once

#include <vector>

#include "my_engine/Component.h"
#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/physics/RigidBody.h"

class StageController : public Component
{
public:
    StageController(GameObject* owner, std::vector<RigidBody*>& rigidBodiesRef, bool& resetRequestedRef);

    void Update(float deltaTime) override;

private:
    std::vector<RigidBody*>& rigidBodies;
    bool& resetRequested;
};
