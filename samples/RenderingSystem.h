#pragma once

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl2.h"

#include "my_engine/Component.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/RigidBody.h"
#include "my_engine/physics/JointComponent.h"

void DrawBody(const physics::Body& body);
void DrawJoint(const physics::Joint& joint);

class BodyRenderer : public Component
{
public:
    BodyRenderer(GameObject* owner, const RigidBody& rigidBodyRef);

    void Update(float deltaTime) override;

private:
    const RigidBody& rigidBody;
};

class JointRenderer : public Component
{
public:
    JointRenderer(GameObject* owner, const JointComponent& jointComponent);

    void Update(float deltaTime) override;

private:
    const JointComponent& jointComponent;
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
