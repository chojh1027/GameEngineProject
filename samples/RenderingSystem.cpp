#include "RenderingSystem.h"

#include <cstdio>

#include "glad/glad.h"

namespace
{
constexpr int kCircleSegments = 32;
constexpr float kOverlayColor[4] = {230.0f / 255.0f, 153.0f / 255.0f, 153.0f / 255.0f, 1.0f};

void DrawText(int x, int y, const char* string)
{
    ImVec2 p;
    p.x = static_cast<float>(x);
    p.y = static_cast<float>(y);
    ImGui::Begin("Overlay", NULL,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(p);
    ImGui::TextColored(ImColor(kOverlayColor[0], kOverlayColor[1], kOverlayColor[2], kOverlayColor[3]), "%s", string);
    ImGui::End();
}
} // namespace

void DrawBody(const physics::Body& body)
{
    Mat22 R(body.rotation);
    Vec2 x = body.position;
    Vec2 h = 0.5f * body.width;

    glColor3f(0.8f, 0.8f, 0.9f);

    if (body.shape == physics::Body::ShapeType::Circle)
    {
        float radius = body.radius;

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < kCircleSegments; ++i)
        {
            float angle = 2.0f * k_pi * (static_cast<float>(i) / static_cast<float>(kCircleSegments));
            float c = cosf(angle);
            float s = sinf(angle);
            Vec2 vertex = x + Vec2(c * radius, s * radius);
            glVertex2f(vertex.x, vertex.y);
        }
        glEnd();
        return;
    }

    Vec2 v1 = x + R * Vec2(-h.x, -h.y);
    Vec2 v2 = x + R * Vec2(h.x, -h.y);
    Vec2 v3 = x + R * Vec2(h.x, h.y);
    Vec2 v4 = x + R * Vec2(-h.x, h.y);

    glBegin(GL_LINE_LOOP);
    glVertex2f(v1.x, v1.y);
    glVertex2f(v2.x, v2.y);
    glVertex2f(v3.x, v3.y);
    glVertex2f(v4.x, v4.y);
    glEnd();
}

void DrawJoint(const physics::Joint& joint)
{
    if (joint.body1 == nullptr || joint.body2 == nullptr)
        return;

    Mat22 R1(joint.body1->rotation);
    Mat22 R2(joint.body2->rotation);

    Vec2 x1 = joint.body1->position;
    Vec2 p1 = x1 + R1 * joint.localAnchor1;

    Vec2 x2 = joint.body2->position;
    Vec2 p2 = x2 + R2 * joint.localAnchor2;

    glColor3f(0.5f, 0.5f, 0.8f);
    glBegin(GL_LINES);
    glVertex2f(x1.x, x1.y);
    glVertex2f(p1.x, p1.y);
    glVertex2f(x2.x, x2.y);
    glVertex2f(p2.x, p2.y);
    glEnd();
}

BodyRenderer::BodyRenderer(GameObject* owner, const RigidBody& rigidBodyRef)
    : Component(owner)
    , rigidBody(rigidBodyRef)
{
}

void BodyRenderer::Update(float deltaTime)
{
    (void)deltaTime;

    const physics::Body* body = rigidBody.GetBody();
    if (body == nullptr)
        return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawBody(*body);
}

JointRenderer::JointRenderer(GameObject* owner, const JointComponent& jointComponent)
    : Component(owner)
    , jointComponent(jointComponent)
{
}

void JointRenderer::Update(float deltaTime)
{
    (void)deltaTime;

    const physics::Joint* joint = jointComponent.GetJoint();
    if (joint == nullptr)
        return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawJoint(*joint);
}

FrameRenderer::FrameRenderer(GLFWwindow* windowPtr)
    : window(windowPtr)
{
}

bool FrameRenderer::Initialize()
{
    if (window == nullptr)
        return false;

    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float uiScale = xscale;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = uiScale;

    return true;
}

bool FrameRenderer::BeginFrame()
{
    if (window == nullptr)
        return false;

    if (glfwWindowShouldClose(window))
        return false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    return true;
}

void FrameRenderer::RenderOverlay(float deltaTime) const
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Delta Time: %.3f", deltaTime);

    DrawText(5, 5, "Joint Chain Stage");
    DrawText(5, 35, "Keys: R Reset, A Accumulation, P Position Correction, W Warm Starting");
    DrawText(5, 65, buffer);
}

void FrameRenderer::FinishFrame() const
{
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glfwPollEvents();
    glfwSwapBuffers(window);
}

void FrameRenderer::Shutdown() const
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
