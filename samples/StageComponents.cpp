#include "StageComponents.h"

StageController::StageController(GameObject* owner, std::vector<RigidBody*>& rigidBodiesRef, bool& resetRequestedRef)
    : Component(owner)
    , rigidBodies(rigidBodiesRef)
    , resetRequested(resetRequestedRef)
{
}

void StageController::Update(float deltaTime)
{
    (void)deltaTime;

    if (!resetRequested)
        return;

    for (RigidBody* body : rigidBodies)
    {
        if (body == nullptr)
            continue;

        body->Reset();
    }

    if (gPhysicsManager != nullptr)
        gPhysicsManager->RebuildWorld();

    resetRequested = false;
}
