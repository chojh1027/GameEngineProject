/*
 * Simplified box2d-lite sample that only runs the circle stage.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <float.h>
#include <map>
#include <math.h>
#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl2.h"

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "my_engine/physics/World.h"
#include "my_engine/physics/Body.h"
#include "my_engine/physics/Joint.h"
#include "my_engine/Component.h"
#include "my_engine/GameLoop.h"
#include "my_engine/GameObject.h"

namespace
{
using physics::Body;
using physics::Arbiter;
using physics::ArbiterKey;
using physics::World;

GLFWwindow* mainWindow = NULL;

physics::Body bodies[200];

float timeStep = 1.0f / 60.0f;
int iterations = 10;
Vec2 gravity(0.0f, -10.0f);

int numBodies = 0;

int width = 1280;
int height = 720;
float zoom = 10.0f;
float pan_y = 8.0f;

physics::World world(gravity, iterations);
}

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

static void DrawBody(Body* body)
{
        Mat22 R(body->rotation);
        Vec2 x = body->position;
        Vec2 h = 0.5f * body->width;

        glColor3f(0.8f, 0.8f, 0.9f);

        if (body->shape == Body::ShapeType::Circle)
        {
                const int segments = 32;
                float radius = body->radius;

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

static void SetupCircleStage(Body* b)
{
        // Ground
        b->Set(Vec2(100.0f, 20.0f), FLT_MAX);
        b->position.Set(0.0f, -0.5f * b->width.y);
        world.Add(b);
        ++b;
        ++numBodies;

        // Circle stack
        for (int i = 0; i < 5; ++i)
        {
                b->Set(Vec2(1.0f, 1.0f), 10.0f, Body::ShapeType::Circle);
                b->position.Set(-6.0f + 3.0f * i, 8.0f);
                world.Add(b);
                ++b;
                ++numBodies;
        }
}

static void InitCircleStage()
{
        world.Clear();
        numBodies = 0;

        SetupCircleStage(bodies);
}

static void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        if (action != GLFW_PRESS)
        {
                return;
        }

        switch (key)
        {
        case GLFW_KEY_ESCAPE:
                // Quit
                glfwSetWindowShouldClose(mainWindow, GL_TRUE);
                break;

        case GLFW_KEY_A:
                World::accumulateImpulses = !World::accumulateImpulses;
                break;

        case GLFW_KEY_P:
                World::positionCorrection = !World::positionCorrection;
                break;

        case GLFW_KEY_W:
                World::warmStarting = !World::warmStarting;
                break;

        case GLFW_KEY_R:
                InitCircleStage();
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

class Box2DSampleComponent : public Component
{
public:
        Box2DSampleComponent(GameObject* owner, GameLoop& loopRef, GLFWwindow* windowPtr)
                : Component(owner)
                , loop(loopRef)
                , window(windowPtr)
        {
        }

        void Update(float deltaTime) override
        {
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

                ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
                ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
                ImGui::End();

                DrawText(5, 5, "Circle Stage");
                DrawText(5, 35, "Keys: R Reset, A Accumulation, P Position Correction, W Warm Starting");

                char buffer[64];
                snprintf(buffer, sizeof(buffer), "Delta Time: %.3f", deltaTime);
                DrawText(5, 65, buffer);

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                const float step = deltaTime > 0.0f ? deltaTime : timeStep;
                world.Step(step);

                for (int i = 0; i < numBodies; ++i)
                        DrawBody(bodies + i);

                glPointSize(4.0f);
                glColor3f(1.0f, 0.0f, 0.0f);
                glBegin(GL_POINTS);
                std::map<ArbiterKey, Arbiter>::const_iterator iter;
                for (iter = world.arbiters.begin(); iter != world.arbiters.end(); ++iter)
                {
                        const Arbiter& arbiter = iter->second;
                        for (int i = 0; i < arbiter.numContacts; ++i)
                        {
                                Vec2 p = arbiter.contacts[i].position;
                                glVertex2f(p.x, p.y);
                        }
                }
                glEnd();
                glPointSize(1.0f);

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
        GameLoop& loop;
        GLFWwindow* window;
};

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

        InitCircleStage();

        GameLoop gameLoop;
        GameObject gameWorld;
        Box2DSampleComponent sampleComponent(&gameWorld, gameLoop, mainWindow);
        gameWorld.AddComponent(&sampleComponent);

        if (!gameLoop.AddGameObject(&gameWorld))
        {
                fprintf(stderr, "Failed to register game object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        gameLoop.Run();

        glfwTerminate();
        return 0;
}
