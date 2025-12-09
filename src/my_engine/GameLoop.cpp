#include "my_engine/GameLoop.h"

#include <algorithm>
#include <chrono>
#include <iterator>

#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/InputSystem.h"
#include "../../samples/RenderingSystem.h"
#include "my_engine/physics/RigidBody.h"

GameLoop::GameLoop()
{
        std::fill(std::begin(gameObjects), std::end(gameObjects), nullptr);
}

void GameLoop::SetPhysicsManager(PhysicsManager* manager)
{
        physicsManager = manager;
        gPhysicsManager = manager;
}

void GameLoop::SetRenderer(FrameRenderer* renderer)
{
        frameRenderer = renderer;
}

void GameLoop::SetInputSystem(InputSystem* system)
{
        inputSystem = system;
        gInputSystem = system;
}

bool GameLoop::AddGameObject(GameObject* object)
{
        if (object == nullptr || gameObjectCount >= MAX_GAMEOBJECT_COUNT)
                return false;

        for (int i = 0; i < gameObjectCount; ++i)
                if (gameObjects[i] == object)
                        return false;

        gameObjects[gameObjectCount] = object;
        ++gameObjectCount;
        return true;
}

bool GameLoop::RemoveGameObject(GameObject* object)
{
        if (object == nullptr)
                return false;

        for (int i = 0; i < gameObjectCount; ++i)
        {
                if (gameObjects[i] != object)
                        continue;

                for (int j = i; j < gameObjectCount - 1; ++j)
                        gameObjects[j] = gameObjects[j + 1];

                gameObjects[gameObjectCount - 1] = nullptr;
                --gameObjectCount;
                return true;
        }

        return false;
}

void GameLoop::Run()
{
        if (isRunning || gameObjectCount == 0)
                return;

        isRunning = true;

        InitializeObjects();

        if (frameRenderer != nullptr && !frameRenderer->Initialize())
        {
                isRunning = false;
                ShutdownObjects();
                ClearObjects();
                return;
        }

        auto previous = std::chrono::steady_clock::now();
        float accumulator = 0.0f;

        while (isRunning)
        {
                auto current = std::chrono::steady_clock::now();
                std::chrono::duration<float> delta = current - previous;
                previous = current;

                if (frameRenderer != nullptr && !frameRenderer->BeginFrame())
                {
                        Stop();
                        break;
                }

                if (inputSystem != nullptr)
                        inputSystem->Update();

                accumulator += delta.count();
                while (accumulator >= fixedDeltaTime)
                {
                        FixedUpdateObjects(fixedDeltaTime);
                        accumulator -= fixedDeltaTime;
                }

                UpdateObjects(delta.count());

                if (frameRenderer != nullptr)
                {
                        frameRenderer->RenderOverlay(delta.count());
                        frameRenderer->FinishFrame();
                }

                if (gameObjectCount == 0)
                        isRunning = false;
        }

        if (frameRenderer != nullptr)
                frameRenderer->Shutdown();

        ShutdownObjects();
        ClearObjects();
}

void GameLoop::Stop()
{
        isRunning = false;
}

void GameLoop::InitializeObjects()
{
        for (int i = 0; i < gameObjectCount; ++i)
        {
                if (gameObjects[i] == nullptr)
                        continue;

                gameObjects[i]->Init();
        }

        for (int i = 0; i < gameObjectCount; ++i)
        {
                if (gameObjects[i] == nullptr)
                        continue;

                gameObjects[i]->Start();
        }
}

void GameLoop::UpdateObjects(float deltaTime)
{
for (int i = 0; i < gameObjectCount; ++i)
{
if (gameObjects[i] == nullptr)
continue;

gameObjects[i]->Update(deltaTime);
}
}

void GameLoop::FixedUpdateObjects(float deltaTime)
{
if (physicsManager != nullptr)
physicsManager->Step(deltaTime);

for (int i = 0; i < gameObjectCount; ++i)
{
if (gameObjects[i] == nullptr)
continue;

gameObjects[i]->FixedUpdate(deltaTime);
}
}

void GameLoop::ShutdownObjects()
{
        for (int i = 0; i < gameObjectCount; ++i)
        {
                if (gameObjects[i] == nullptr)
                        continue;

                gameObjects[i]->Destroy();
        }
}

void GameLoop::ClearObjects()
{
        std::fill(std::begin(gameObjects), std::end(gameObjects), nullptr);
        gameObjectCount = 0;
}
