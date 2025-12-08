#pragma once

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl2.h"

#include "my_engine/Component.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/RigidBody.h"

void DrawBody(const physics::Body& body);

class BodyRenderer : public Component
{
public:
    BodyRenderer(GameObject* owner, const RigidBody& rigidBodyRef);

    void Update(float deltaTime) override;

private:
    const RigidBody& rigidBody;
};

class FrameRenderer
{
public:
    explicit FrameRenderer(GLFWwindow* windowPtr);

    bool Initialize();
    bool BeginFrame();
    void RenderOverlay(float deltaTime) const;
    void FinishFrame() const;
    void Shutdown() const;

private:
    GLFWwindow* window;
};
