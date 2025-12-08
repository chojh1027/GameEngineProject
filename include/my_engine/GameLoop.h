#pragma once
#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "my_engine/Constant.h"
#include "my_engine/GameObject.h"

#include <functional>

class PhysicsManager;

class GameLoop {
public:
        GameLoop();

        bool AddGameObject(GameObject* object);
        bool RemoveGameObject(GameObject* object);
        void SetPhysicsManager(PhysicsManager* manager);
        void SetPreFrameCallback(std::function<bool()> callback) { preFrameCallback = std::move(callback); }
        void SetPostFrameCallback(std::function<void(float)> callback) { postFrameCallback = std::move(callback); }
        void SetShutdownCallback(std::function<void()> callback) { shutdownCallback = std::move(callback); }
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
bool isRunning = false;
float fixedDeltaTime = 1.0f / 60.0f;
std::function<bool()> preFrameCallback;
std::function<void(float)> postFrameCallback;
std::function<void()> shutdownCallback;
};

#endif // GAME_LOOP_H
