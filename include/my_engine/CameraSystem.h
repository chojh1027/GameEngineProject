#pragma once

#include "box2d-lite/MathUtils.h"

struct GLFWwindow;

class CameraSystem
{
public:
    bool Initialize(GLFWwindow* windowPtr);

    void Update(float deltaTime);
    void ApplyView() const;
    void OnResize(int newWidth, int newHeight);

    Vec2 ScreenToWorld(double mouseX, double mouseY, int screenWidth, int screenHeight) const;

    void SetPosition(const Vec2& newPosition) { position = newPosition; }
    Vec2 GetPosition() const { return position; }

    void SetZoom(float newZoom);
    float GetZoom() const { return zoom; }

    void SetMoveSpeed(float speed) { moveSpeed = speed; }
    void SetZoomSpeed(float speed) { zoomSpeed = speed; }

private:
    GLFWwindow* window = nullptr;
    int width = 0;
    int height = 0;

    float zoom = 10.0f;
    Vec2 position = Vec2(0.0f, 8.0f);

    float moveSpeed = 10.0f;
    float zoomSpeed = 5.0f;
    float minZoom = 1.0f;
};

extern CameraSystem* gCameraSystem;
