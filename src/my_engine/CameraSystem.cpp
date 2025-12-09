#include "my_engine/CameraSystem.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "my_engine/InputSystem.h"

CameraSystem* gCameraSystem = nullptr;

bool CameraSystem::Initialize(GLFWwindow* windowPtr)
{
    window = windowPtr;
    if (window == nullptr)
        return false;

    glfwGetWindowSize(window, &width, &height);
    OnResize(width, height);
    ApplyView();
    return true;
}

void CameraSystem::Update(float deltaTime)
{
    if (window == nullptr || gInputSystem == nullptr)
        return;

    /*float moveAmount = moveSpeed * deltaTime;
    Vec2 deltaPosition(0.0f, 0.0f);

    if (gInputSystem->IsKeyDown(GLFW_KEY_LEFT))
        deltaPosition.x -= moveAmount;
    if (gInputSystem->IsKeyDown(GLFW_KEY_RIGHT))
        deltaPosition.x += moveAmount;
    if (gInputSystem->IsKeyDown(GLFW_KEY_UP))
        deltaPosition.y += moveAmount;
    if (gInputSystem->IsKeyDown(GLFW_KEY_DOWN))
        deltaPosition.y -= moveAmount;

    position += deltaPosition;

    float zoomChange = zoomSpeed * deltaTime;
    if (gInputSystem->IsKeyDown(GLFW_KEY_Q))
        SetZoom(zoom + zoomChange);
    if (gInputSystem->IsKeyDown(GLFW_KEY_E))
        SetZoom(zoom - zoomChange);*/
}

void CameraSystem::ApplyView() const
{
    if (window == nullptr || height == 0)
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = static_cast<float>(width) / static_cast<float>(height);

    if (width >= height)
    {
        glOrtho(position.x - zoom * aspect, position.x + zoom * aspect, position.y - zoom, position.y + zoom, -1.0, 1.0);
    }
    else
    {
        glOrtho(position.x - zoom, position.x + zoom, position.y - zoom / aspect, position.y + zoom / aspect, -1.0, 1.0);
    }
}

void CameraSystem::OnResize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight > 0 ? newHeight : 1;

    glViewport(0, 0, width, height);
}

Vec2 CameraSystem::ScreenToWorld(double mouseX, double mouseY, int screenWidth, int screenHeight) const
{
    if (screenHeight == 0)
        return position;

    float aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    float viewHalfWidth = (screenWidth >= screenHeight) ? zoom * aspect : zoom;
    float viewHalfHeight = (screenWidth >= screenHeight) ? zoom : zoom / aspect;

    float worldLeft = position.x - viewHalfWidth;
    float worldBottom = position.y - viewHalfHeight;

    float normalizedX = static_cast<float>(mouseX) / static_cast<float>(screenWidth);
    float normalizedY = static_cast<float>(mouseY) / static_cast<float>(screenHeight);

    float worldX = worldLeft + normalizedX * viewHalfWidth * 2.0f;
    float worldY = (position.y + viewHalfHeight) - normalizedY * viewHalfHeight * 2.0f;

    return Vec2(worldX, worldY);
}

void CameraSystem::SetZoom(float newZoom)
{
    zoom = newZoom;
    if (zoom < minZoom)
        zoom = minZoom;
}
