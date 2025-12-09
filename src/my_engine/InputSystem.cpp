#include "my_engine/InputSystem.h"

#include <algorithm>

InputSystem* gInputSystem = nullptr;

bool InputSystem::Initialize(GLFWwindow* windowPtr)
{
    window = windowPtr;
    if (window == nullptr)
        return false;

    keyStates.assign(GLFW_KEY_LAST + 1, GLFW_RELEASE);
    previousKeyStates = keyStates;

    mouseButtonStates.assign(GLFW_MOUSE_BUTTON_LAST + 1, GLFW_RELEASE);
    previousMouseButtonStates = mouseButtonStates;

    glfwGetCursorPos(window, &mouseX, &mouseY);
    return true;
}

void InputSystem::Update()
{
    if (window == nullptr)
        return;

    previousKeyStates = keyStates;
    previousMouseButtonStates = mouseButtonStates;

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
        keyStates[key] = glfwGetKey(window, key);

    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
        mouseButtonStates[button] = glfwGetMouseButton(window, button);

    glfwGetCursorPos(window, &mouseX, &mouseY);
}

bool InputSystem::IsKeyDown(int key) const
{
    if (!IsValidKey(key))
        return false;

    return keyStates[key] == GLFW_PRESS;
}

bool InputSystem::WasKeyPressed(int key) const
{
    if (!IsValidKey(key))
        return false;

    return keyStates[key] == GLFW_PRESS && previousKeyStates[key] == GLFW_RELEASE;
}

bool InputSystem::WasKeyReleased(int key) const
{
    if (!IsValidKey(key))
        return false;

    return keyStates[key] == GLFW_RELEASE && previousKeyStates[key] == GLFW_PRESS;
}

bool InputSystem::IsMouseDown(int button) const
{
    if (!IsValidMouseButton(button))
        return false;

    return mouseButtonStates[button] == GLFW_PRESS;
}

bool InputSystem::WasMousePressed(int button) const
{
    if (!IsValidMouseButton(button))
        return false;

    return mouseButtonStates[button] == GLFW_PRESS && previousMouseButtonStates[button] == GLFW_RELEASE;
}

bool InputSystem::WasMouseReleased(int button) const
{
    if (!IsValidMouseButton(button))
        return false;

    return mouseButtonStates[button] == GLFW_RELEASE && previousMouseButtonStates[button] == GLFW_PRESS;
}

void InputSystem::RequestClose() const
{
    if (window == nullptr)
        return;

    glfwSetWindowShouldClose(window, GL_TRUE);
}

bool InputSystem::IsValidKey(int key) const
{
    return key >= 0 && key <= GLFW_KEY_LAST;
}

bool InputSystem::IsValidMouseButton(int button) const
{
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST;
}
