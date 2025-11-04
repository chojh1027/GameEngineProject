#pragma once
#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "my_engine/Constant.h"
#include "my_engine/GameObject.h"

class GameLoop {
public:
        GameLoop();

        bool AddGameObject(GameObject* object);
        bool RemoveGameObject(GameObject* object);
        void Run();
        void Stop();

        bool IsRunning() const { return isRunning; }

private:
        void InitializeObjects();
        void UpdateObjects(float deltaTime);
        void ShutdownObjects();
        void ClearObjects();

        GameObject* gameObjects[MAX_GAMEOBJECT_COUNT];
        int gameObjectCount = 0;
        bool isRunning = false;
};

#endif // GAME_LOOP_H
