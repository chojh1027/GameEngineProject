#include "my_engine/GameLoop.h"

#include <algorithm>
#include <chrono>
#include <iterator>

GameLoop::GameLoop()
{
        std::fill(std::begin(gameObjects), std::end(gameObjects), nullptr);
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

        auto previous = std::chrono::steady_clock::now();

        while (isRunning)
        {
                auto current = std::chrono::steady_clock::now();
                std::chrono::duration<float> delta = current - previous;
                previous = current;

                UpdateObjects(delta.count());

                if (gameObjectCount == 0)
                        isRunning = false;
        }

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
