#pragma once
#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "my_engine/Constant.h"
#include "my_engine/GameObject.h"

#include <vector>

class PhysicsManager;
class FrameRenderer;
class RigidBody;

class GameLoop {
public:
        GameLoop();

        bool AddGameObject(GameObject* object);
        bool RemoveGameObject(GameObject* object);
        void SetPhysicsManager(PhysicsManager* manager);
        void SetRenderer(FrameRenderer* renderer);
        void SetResetTargets(std::vector<RigidBody*>* bodies, bool* resetFlag);
        void Run();
        void Stop();

        bool IsRunning() const { return isRunning; }

private:
void InitializeObjects();
void FixedUpdateObjects(float fixedDeltaTime);
void UpdateObjects(float deltaTime);
void ShutdownObjects();
void ClearObjects();

GameObject* gameObjects[MAX_GAMEOBJECT_COUNT];
int gameObjectCount = 0;
PhysicsManager* physicsManager = nullptr;
FrameRenderer* frameRenderer = nullptr;
std::vector<RigidBody*>* resetBodies = nullptr;
bool* resetRequested = nullptr;
bool isRunning = false;
float fixedDeltaTime = 1.0f / 60.0f;
};

#endif // GAME_LOOP_H
