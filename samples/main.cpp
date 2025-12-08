/*
 * Simplified box2d-lite sample that only runs the circle stage.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <float.h>
#include <math.h>
#include <memory>
#include <stdio.h>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl2.h"

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "my_engine/Component.h"
#include "my_engine/GameLoop.h"
#include "my_engine/GameObject.h"
#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/physics/RigidBody.h"

namespace
{
using physics::Body;

constexpr int kCircleStackCount = 5;
constexpr float kGroundWidth = 100.0f;
constexpr float kGroundHeight = 20.0f;
constexpr float kGroundYOffset = -0.5f;
constexpr float kCircleRadius = 1.0f;
constexpr float kCircleMass = 10.0f;
constexpr float kCircleStartY = 8.0f;
constexpr float kCircleStartX = -6.0f;
constexpr float kCircleSpacing = 3.0f;

GLFWwindow* mainWindow = NULL;

float zoom = 10.0f;
float pan_y = 8.0f;
int width = 1280;
int height = 720;

bool gResetRequested = false;
} // namespace

static void glfwErrorCallback(int error, const char* description)
{
        printf("GLFW error %d: %s\n", error, description);
}

static void DrawText(int x, int y, const char* string)
{
        ImVec2 p;
        p.x = float(x);
        p.y = float(y);
        ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPos(p);
        ImGui::TextColored(ImColor(230, 153, 153, 255), "%s", string);
        ImGui::End();
}

static void DrawBody(const Body& body)
{
        Mat22 R(body.rotation);
        Vec2 x = body.position;
        Vec2 h = 0.5f * body.width;

        glColor3f(0.8f, 0.8f, 0.9f);

        if (body.shape == Body::ShapeType::Circle)
        {
                const int segments = 32;
                float radius = body.radius;

                glBegin(GL_LINE_LOOP);
                for (int i = 0; i < segments; ++i)
                {
                        float angle = 2.0f * k_pi * (static_cast<float>(i) / static_cast<float>(segments));
                        float c = cosf(angle);
                        float s = sinf(angle);
                        Vec2 vertex = x + Vec2(c * radius, s * radius);
                        glVertex2f(vertex.x, vertex.y);
                }
                glEnd();
                return;
        }

        Vec2 v1 = x + R * Vec2(-h.x, -h.y);
        Vec2 v2 = x + R * Vec2( h.x, -h.y);
        Vec2 v3 = x + R * Vec2( h.x,  h.y);
        Vec2 v4 = x + R * Vec2(-h.x,  h.y);

        glBegin(GL_LINE_LOOP);
        glVertex2f(v1.x, v1.y);
        glVertex2f(v2.x, v2.y);
        glVertex2f(v3.x, v3.y);
        glVertex2f(v4.x, v4.y);
        glEnd();
}

static void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        (void)window;
        (void)scancode;
        (void)mods;

        if (action != GLFW_PRESS)
        {
                return;
        }

        switch (key)
        {
        case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(mainWindow, GL_TRUE);
                break;

        case GLFW_KEY_A:
                physics::World::accumulateImpulses = !physics::World::accumulateImpulses;
                break;

        case GLFW_KEY_P:
                physics::World::positionCorrection = !physics::World::positionCorrection;
                break;

        case GLFW_KEY_W:
                physics::World::warmStarting = !physics::World::warmStarting;
                break;

        case GLFW_KEY_R:
                gResetRequested = true;
                break;
        }
}

static void Reshape(GLFWwindow*, int w, int h)
{
        width = w;
        height = h > 0 ? h : 1;

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        float aspect = float(width) / float(height);
        if (width >= height)
        {
                // aspect >= 1, set the height from -1 to 1, with larger width
                glOrtho(-zoom * aspect, zoom * aspect, -zoom + pan_y, zoom + pan_y, -1.0, 1.0);
        }
        else
        {
                // aspect < 1, set the width to -1 to 1, with larger height
                glOrtho(-zoom, zoom, -zoom / aspect + pan_y, zoom / aspect + pan_y, -1.0, 1.0);
        }
}

class FrameControllerComponent : public Component
{
public:
        FrameControllerComponent(GameObject* owner, GameLoop& loopRef, GLFWwindow* windowPtr)
                : Component(owner)
                , loop(loopRef)
                , window(windowPtr)
        {
        }

        void Update(float deltaTime) override
        {
                (void)deltaTime;

                if (window == nullptr)
                {
                        loop.Stop();
                        return;
                }

                if (glfwWindowShouldClose(window))
                {
                        loop.Stop();
                        return;
                }

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ImGui_ImplOpenGL2_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
        }

private:
        GameLoop& loop;
        GLFWwindow* window;
};

class BodyRenderer : public Component
{
public:
        BodyRenderer(GameObject* owner, const RigidBody& rigidBodyRef)
                : Component(owner)
                , rigidBody(rigidBodyRef)
        {
        }

        void Update(float deltaTime) override
        {
                (void)deltaTime;

                const physics::Body* body = rigidBody.GetBody();
                if (body == nullptr)
                        return;

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                DrawBody(*body);
        }

private:
        const RigidBody& rigidBody;
};

class UIRenderer : public Component
{
public:
        UIRenderer(GameObject* owner, GLFWwindow* windowPtr)
                : Component(owner)
                , window(windowPtr)
        {
        }

        void Update(float deltaTime) override
        {
                DrawText(5, 5, "Circle Stage");
                DrawText(5, 35, "Keys: R Reset, A Accumulation, P Position Correction, W Warm Starting");

                char buffer[64];
                snprintf(buffer, sizeof(buffer), "Delta Time: %.3f", deltaTime);
                DrawText(5, 65, buffer);

                ImGui::Render();
                ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

                glfwPollEvents();
                glfwSwapBuffers(window);
        }

        void Destroy() override
        {
                ImGui_ImplOpenGL2_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
        }

private:
        GLFWwindow* window;
};

class StageController : public Component
{
public:
        StageController(GameObject* owner, PhysicsManager& manager, std::vector<RigidBody*>& rigidBodiesRef)
                : Component(owner)
                , physicsManager(manager)
                , rigidBodies(rigidBodiesRef)
        {
        }

        void Update(float deltaTime) override
        {
                (void)deltaTime;

                if (!gResetRequested)
                        return;

                for (RigidBody* body : rigidBodies)
                {
                        if (body == nullptr)
                                continue;

                        body->Reset();
                }

                physicsManager.RebuildWorld();
                gResetRequested = false;
        }

private:
        PhysicsManager& physicsManager;
        std::vector<RigidBody*>& rigidBodies;
};

struct StageObject
{
        std::unique_ptr<GameObject> object;
        std::vector<std::unique_ptr<Component>> components;
        RigidBody* rigidBody = nullptr;
};

static StageObject CreateBodyObject(PhysicsManager& physicsManager,
                                    physics::Body::ShapeType shape,
                                    const Vec2& size,
                                    float mass,
                                    const Vec2& position)
{
        StageObject stageObject;
        stageObject.object = std::make_unique<GameObject>();

        auto rigidBody = std::make_unique<RigidBody>(stageObject.object.get(), physicsManager, size, mass, shape, position);
        stageObject.rigidBody = rigidBody.get();
        stageObject.object->AddComponent(stageObject.rigidBody);

        auto renderer = std::make_unique<BodyRenderer>(stageObject.object.get(), *stageObject.rigidBody);
        stageObject.object->AddComponent(renderer.get());

        stageObject.components.push_back(std::move(rigidBody));
        stageObject.components.push_back(std::move(renderer));

        return stageObject;
}

static void BuildCircleStage(PhysicsManager& physicsManager,
                             std::vector<StageObject>& stageObjects,
                             std::vector<RigidBody*>& stageBodies)
{
        stageObjects.clear();
        stageBodies.clear();
        stageObjects.reserve(static_cast<size_t>(kCircleStackCount) + 1);
        stageBodies.reserve(static_cast<size_t>(kCircleStackCount) + 1);

        StageObject ground = CreateBodyObject(physicsManager,
                                              physics::Body::ShapeType::Box,
                                              Vec2(kGroundWidth, kGroundHeight),
                                              FLT_MAX,
                                              Vec2(0.0f, kGroundYOffset * kGroundHeight));
        stageBodies.push_back(ground.rigidBody);
        stageObjects.push_back(std::move(ground));

        for (int i = 0; i < kCircleStackCount; ++i)
        {
                Vec2 position(kCircleStartX + kCircleSpacing * i, kCircleStartY);
                StageObject circle = CreateBodyObject(physicsManager,
                                                      physics::Body::ShapeType::Circle,
                                                      Vec2(kCircleRadius, kCircleRadius),
                                                      kCircleMass,
                                                      position);
                stageBodies.push_back(circle.rigidBody);
                stageObjects.push_back(std::move(circle));
        }
}

int main(int, char**)
{
        glfwSetErrorCallback(glfwErrorCallback);

        if (glfwInit() == 0)
        {
                fprintf(stderr, "Failed to initialize GLFW\n");
                return -1;
        }

        mainWindow = glfwCreateWindow(width, height, "box2d-lite", NULL, NULL);
        if (mainWindow == NULL)
        {
                fprintf(stderr, "Failed to open GLFW mainWindow.\n");
                glfwTerminate();
                return -1;
        }

        glfwMakeContextCurrent(mainWindow);

        // Load OpenGL functions using glad
        int gladStatus = gladLoadGL();
        if (gladStatus == 0)
        {
                fprintf(stderr, "Failed to load OpenGL.\n");
                glfwTerminate();
                return -1;
        }

        glfwSwapInterval(1);
        glfwSetWindowSizeCallback(mainWindow, Reshape);
        glfwSetKeyCallback(mainWindow, Keyboard);

        float xscale, yscale;
        glfwGetWindowContentScale(mainWindow, &xscale, &yscale);
        float uiScale = xscale;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsClassic();
        ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
        ImGui_ImplOpenGL2_Init();
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = uiScale;

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        float aspect = float(width) / float(height);
        if (width >= height)
        {
                // aspect >= 1, set the height from -1 to 1, with larger width
                glOrtho(-zoom * aspect, zoom * aspect, -zoom + pan_y, zoom + pan_y, -1.0, 1.0);
        }
        else
        {
                // aspect < 1, set the width to -1 to 1, with larger height
                glOrtho(-zoom, zoom, -zoom / aspect + pan_y, zoom / aspect + pan_y, -1.0, 1.0);
        }

        PhysicsManager physicsManager(Vec2(0.0f, -10.0f), 10);

        GameLoop gameLoop;
        gameLoop.SetPhysicsManager(&physicsManager);

        GameObject frameControllerObject;
        FrameControllerComponent frameController(&frameControllerObject, gameLoop, mainWindow);
        frameControllerObject.AddComponent(&frameController);

        std::vector<StageObject> stageObjects;
        std::vector<RigidBody*> stageBodies;

        GameObject stageControllerObject;
        StageController stageController(&stageControllerObject, physicsManager, stageBodies);
        stageControllerObject.AddComponent(&stageController);

        BuildCircleStage(physicsManager, stageObjects, stageBodies);

        GameObject uiObject;
        UIRenderer uiRenderer(&uiObject, mainWindow);
        uiObject.AddComponent(&uiRenderer);

        if (!gameLoop.AddGameObject(&frameControllerObject))
        {
                fprintf(stderr, "Failed to register frame controller with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        if (!gameLoop.AddGameObject(&stageControllerObject))
        {
                fprintf(stderr, "Failed to register stage controller with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        for (StageObject& stageObject : stageObjects)
        {
                if (!gameLoop.AddGameObject(stageObject.object.get()))
                {
                        fprintf(stderr, "Failed to register stage object with the game loop.\n");
                        glfwTerminate();
                        return -1;
                }
        }

        if (!gameLoop.AddGameObject(&uiObject))
        {
                fprintf(stderr, "Failed to register UI renderer with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        gameLoop.Run();

        glfwTerminate();
        return 0;
}
