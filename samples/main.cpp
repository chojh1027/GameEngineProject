/*
 * Simplified box2d-lite sample that only runs the circle stage.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <float.h>
#include <math.h>
#include <memory>
#include <stdio.h>
#include <vector>


#include "RenderingSystem.h"
#include "my_engine/Component.h"
#include "my_engine/GameLoop.h"
#include "my_engine/GameObject.h"
#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/physics/RigidBody.h"
#include "my_engine/physics/JointComponent.h"

namespace
{
using physics::Body;

constexpr int kLinkCount = 6;
constexpr float kGroundWidth = 100.0f;
constexpr float kGroundHeight = 20.0f;
constexpr float kGroundYOffset = -0.5f;
constexpr float kLinkSpacing = 3.0f;
constexpr float kStartX = -7.5f;
constexpr float kStartY = 10.0f;
constexpr float kBoxSize = 1.0f;
constexpr float kCircleRadius = 1.0f;
constexpr float kDynamicMass = 5.0f;

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
        PhysicsManager physicsManager(Vec2(0.0f, -10.0f), 10);

        GameLoop gameLoop;
        gameLoop.SetPhysicsManager(&physicsManager);
        gameLoop.SetRenderer(&frameRenderer);

        std::vector<std::unique_ptr<GameObject>> ownedObjects;
        std::vector<std::unique_ptr<Component>> ownedComponents;
        std::vector<std::unique_ptr<JointComponent>> ownedJointComponents;
        std::vector<std::unique_ptr<Component>> ownedJointRenderers;
        std::vector<RigidBody*> stageBodies;
        std::vector<RigidBody*> chainBodies;
        std::vector<GameObject*> chainObjects;

        gameLoop.SetResetTargets(&stageBodies, &gResetRequested);

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
        chainBodies.push_back(groundBody.get());
        chainObjects.push_back(groundObject.get());

        if (!gameLoop.AddGameObject(groundObject.get()))
        {
                fprintf(stderr, "Failed to register ground object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        ownedComponents.push_back(std::move(groundBody));
        ownedComponents.push_back(std::move(groundRenderer));
        ownedObjects.push_back(std::move(groundObject));

        for (int i = 0; i < kLinkCount; ++i)
        {
                bool useCircle = (i % 2) == 0;
                Vec2 size = useCircle ? Vec2(kCircleRadius, kCircleRadius) : Vec2(kBoxSize, kBoxSize);
                physics::Body::ShapeType shape = useCircle ? physics::Body::ShapeType::Circle : physics::Body::ShapeType::Box;
                Vec2 position(kStartX + kLinkSpacing * i, kStartY);

                auto linkObject = std::make_unique<GameObject>();
                auto linkBody = std::make_unique<RigidBody>(linkObject.get(), size, kDynamicMass, shape, position);
                auto linkRenderer = std::make_unique<BodyRenderer>(linkObject.get(), *linkBody);
                linkObject->AddComponent(linkBody.get());
                linkObject->AddComponent(linkRenderer.get());
                stageBodies.push_back(linkBody.get());
                chainBodies.push_back(linkBody.get());
                chainObjects.push_back(linkObject.get());

                if (!gameLoop.AddGameObject(linkObject.get()))
                {
                        fprintf(stderr, "Failed to register stage object with the game loop.\n");
                        glfwTerminate();
                        return -1;
                }

                ownedComponents.push_back(std::move(linkBody));
                ownedComponents.push_back(std::move(linkRenderer));
                ownedObjects.push_back(std::move(linkObject));
        }

        for (size_t i = 1; i < chainBodies.size(); ++i)
        {
                physics::Body* bodyA = chainBodies[i - 1]->GetBody();
                physics::Body* bodyB = chainBodies[i]->GetBody();
                Vec2 anchor = (bodyA->position + bodyB->position) * 0.5f;

                GameObject* owningObject = chainObjects[i];
                auto jointComponent = std::make_unique<JointComponent>(owningObject, chainBodies[i - 1], chainBodies[i], anchor);
                auto jointRenderer = std::make_unique<JointRenderer>(owningObject, *jointComponent);

                owningObject->AddComponent(jointComponent.get());
                owningObject->AddComponent(jointRenderer.get());

                ownedJointComponents.push_back(std::move(jointComponent));
                ownedJointRenderers.push_back(std::move(jointRenderer));
        }

        gameLoop.Run();

        glfwTerminate();
        return 0;
}
