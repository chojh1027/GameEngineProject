#pragma once
#ifndef MY_ENGINE_INPUT_SYSTEM_H
#define MY_ENGINE_INPUT_SYSTEM_H

#include <vector>

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

class InputSystem
{
public:
    InputSystem() = default;

    bool Initialize(GLFWwindow* windowPtr);
    void Update();

    bool IsKeyDown(int key) const;
    bool WasKeyPressed(int key) const;
    bool WasKeyReleased(int key) const;

    bool IsMouseDown(int button) const;
    bool WasMousePressed(int button) const;
    bool WasMouseReleased(int button) const;

    double GetMouseX() const { return mouseX; }
    double GetMouseY() const { return mouseY; }

    void RequestClose() const;

private:
    bool IsValidKey(int key) const;
    bool IsValidMouseButton(int button) const;

    GLFWwindow* window = nullptr;
    std::vector<int> keyStates;
    std::vector<int> previousKeyStates;
    std::vector<int> mouseButtonStates;
    std::vector<int> previousMouseButtonStates;
    double mouseX = 0.0;
    double mouseY = 0.0;
};

extern InputSystem* gInputSystem;

#endif // MY_ENGINE_INPUT_SYSTEM_H
