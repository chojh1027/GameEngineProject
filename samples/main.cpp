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

#include "RenderingSystem.h"
#include "StageComponents.h"
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
        Reshape(mainWindow, width, height);

        FrameRenderer frameRenderer(mainWindow);
        if (!frameRenderer.Initialize())
        {
                fprintf(stderr, "Failed to initialize renderer.\n");
                glfwTerminate();
                return -1;
        }

        PhysicsManager physicsManager(Vec2(0.0f, -10.0f), 10);

        GameLoop gameLoop;
        gameLoop.SetPhysicsManager(&physicsManager);

        gameLoop.SetPreFrameCallback([&]() { return frameRenderer.BeginFrame(); });
        gameLoop.SetPostFrameCallback([&](float deltaTime) {
                frameRenderer.RenderOverlay(deltaTime);
                frameRenderer.FinishFrame();
        });
        gameLoop.SetShutdownCallback([&]() { frameRenderer.Shutdown(); });

        std::vector<std::unique_ptr<GameObject>> ownedObjects;
        std::vector<std::unique_ptr<Component>> ownedComponents;
        std::vector<RigidBody*> stageBodies;

        auto stageControllerObject = std::make_unique<GameObject>();
        auto stageController = std::make_unique<StageController>(stageControllerObject.get(), stageBodies, gResetRequested);
        stageControllerObject->AddComponent(stageController.get());

        if (!gameLoop.AddGameObject(stageControllerObject.get()))
        {
                fprintf(stderr, "Failed to register stage controller with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        ownedComponents.push_back(std::move(stageController));
        ownedObjects.push_back(std::move(stageControllerObject));

        auto groundObject = std::make_unique<GameObject>();
        auto groundBody = std::make_unique<RigidBody>(groundObject.get(),
                                                      Vec2(kGroundWidth, kGroundHeight),
                                                      FLT_MAX,
                                                      physics::Body::ShapeType::Box,
                                                      Vec2(0.0f, kGroundYOffset * kGroundHeight));
        auto groundRenderer = std::make_unique<BodyRenderer>(groundObject.get(), *groundBody);
        groundObject->AddComponent(groundBody.get());
        groundObject->AddComponent(groundRenderer.get());
        stageBodies.push_back(groundBody.get());

        if (!gameLoop.AddGameObject(groundObject.get()))
        {
                fprintf(stderr, "Failed to register ground object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        ownedComponents.push_back(std::move(groundBody));
        ownedComponents.push_back(std::move(groundRenderer));
        ownedObjects.push_back(std::move(groundObject));

        for (int i = 0; i < kCircleStackCount; ++i)
        {
                Vec2 position(kCircleStartX + kCircleSpacing * i, kCircleStartY);
                auto circleObject = std::make_unique<GameObject>();
                auto circleBody = std::make_unique<RigidBody>(circleObject.get(),
                                                              Vec2(kCircleRadius, kCircleRadius),
                                                              kCircleMass,
                                                              physics::Body::ShapeType::Circle,
                                                              position);
                auto circleRenderer = std::make_unique<BodyRenderer>(circleObject.get(), *circleBody);
                circleObject->AddComponent(circleBody.get());
                circleObject->AddComponent(circleRenderer.get());
                stageBodies.push_back(circleBody.get());

                if (!gameLoop.AddGameObject(circleObject.get()))
                {
                        fprintf(stderr, "Failed to register stage object with the game loop.\n");
                        glfwTerminate();
                        return -1;
                }

                ownedComponents.push_back(std::move(circleBody));
                ownedComponents.push_back(std::move(circleRenderer));
                ownedObjects.push_back(std::move(circleObject));
        }

        gameLoop.Run();

        glfwTerminate();
        return 0;
}
