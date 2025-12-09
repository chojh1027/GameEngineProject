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
#include "my_engine/InputSystem.h"
#include "my_engine/CameraSystem.h"
#include "my_engine/physics/PhysicsManager.h"
#include "my_engine/physics/RigidBody.h"
#include "my_engine/physics/JointComponent.h"
#include "TextureRenderer.h"

namespace
{
using physics::Body;

constexpr int kLinkCount = 4;
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

int width = 1280;
int height = 720;


const char* playerTextureFile = "metal_ball.bmp";
const char* backgroundTextureFile = "background.bmp";
const char* invironmentRockTextureFile = "rock_texture.bmp";
const char* groundTextureFile = "ground_texture.bmp";


const char* boxTextureFile = "box_texture.png";
const char* linkTextureFile = "chain_link.png";
const char* invironmentTreeTextureFile = "tree_texture.png";

// 배경 위치, 크기
Vec2 backgroundPosition = Vec2(0.0f, kGroundHeight / 2.0f);
Vec2 backgroundSize = Vec2(100.0f, kGroundHeight + 40.0f);

// 바닥 위치, 크기
Vec2 groundPosition = Vec2(0.0f, -kStartY);
Vec2 groundSize = Vec2(kGroundWidth, kGroundHeight);

// 벽 위치, 크기
Vec2 leftWallPosition = Vec2(-kGroundWidth / 2.0f - 1.0f, kGroundHeight / 2.0f);
Vec2 rightWallPosition = Vec2(kGroundWidth / 2.0f + 1.0f, kGroundHeight / 2.0f);
Vec2 wallSize = Vec2(10.0f, kGroundHeight + 20.0f);


// 1: 바위
Vec2 rockPosition = Vec2(5.0f, kGroundYOffset + kGroundHeight / 2.0f + 1.0f);
Vec2 rockSize = Vec2(5.0f, 2.0f);

// 2: 계단
Vec2 step1Position = Vec2(-10.0f, 0.5f);
Vec2 step2Position = Vec2(-8.0f, 1.5f);
Vec2 step3Position = Vec2(-6.0f, 2.5f);
Vec2 step1Size = Vec2(2.0f, 1.0f);
Vec2 step2Size = Vec2(2.0f, 2.0f);
Vec2 step3Size = Vec2(2.0f, 3.0f);

} // namespace

class StageController : public Component
{
public:
        StageController(GameObject* owner, GameLoop* loop)
                : Component(owner)
                , gameLoop(loop)
        {
        }

        void Update(float deltaTime) override
        {
                (void)deltaTime;
                if (gInputSystem == nullptr)
                        return;

                if (gInputSystem->WasKeyPressed(GLFW_KEY_ESCAPE))
                {
                        gInputSystem->RequestClose();
                        if (gameLoop != nullptr)
                                gameLoop->Stop();
                }

                if (gInputSystem->WasKeyPressed(GLFW_KEY_A))
                        physics::World::accumulateImpulses = !physics::World::accumulateImpulses;

                if (gInputSystem->WasKeyPressed(GLFW_KEY_P))
                        physics::World::positionCorrection = !physics::World::positionCorrection;

                if (gInputSystem->WasKeyPressed(GLFW_KEY_W))
                        physics::World::warmStarting = !physics::World::warmStarting;

                if (gInputSystem->WasKeyPressed(GLFW_KEY_R) && gPhysicsManager != nullptr)
                        gPhysicsManager->ResetBodies();
        }

private:
        GameLoop* gameLoop = nullptr;
};

class PlayerController : public Component
{
public:
    PlayerController(GameObject* owner, RigidBody& body)
        : Component(owner)
        , rigidBody(body)
    {
    }

    void Update(float deltaTime) override
    {
        (void)deltaTime;    // Unreferenced parameter
        if (gInputSystem == nullptr)
            return;

        // 카메라를 캐릭터 중심으로 이동
        if (gCameraSystem != nullptr)
        {
            Vec2 playerPos = gameObject->transform->GetPosition();
			gCameraSystem->SetPosition(playerPos + cameraOffset);
		}

        // 마우스 왼쪽 버튼을 누르면 방향을 저장함, 누른 시간을 기록
        // 마우스 왼쪽 버튼을 떼면 저장된 방향으로 누른 시간에 비례하는 힘을 가함
        if (gInputSystem->IsMouseDown(GLFW_MOUSE_BUTTON_LEFT))
        {
            double mouseX = 0.0;
            double mouseY = 0.0;
            glfwGetCursorPos(mainWindow, &mouseX, &mouseY);
            Vec2 worldPos;
            if (gCameraSystem != nullptr)
            {
                worldPos = gCameraSystem->ScreenToWorld(mouseX, mouseY, width, height);
            }
            else
                worldPos = Vec2(0.0f, 0.0f);

            Vec2 playerPos = gameObject->transform->GetPosition();
            aimDirection = Vec2(worldPos.x - playerPos.x, worldPos.y - playerPos.y);
            float length = aimDirection.Length();
            if (length > 0.0f)
                aimDirection = aimDirection / length; // 정규화
            chargeTime += deltaTime;
            if (chargeTime > maxChargeTime)
                chargeTime = maxChargeTime;
        }
        else if (gInputSystem->WasMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
        {
            float chargeRatio = chargeTime / maxChargeTime;
            float forceMagnitude = chargeRatio * forceFactor; // 힘의 크기 계산
            Vec2 force = aimDirection * forceMagnitude;
            rigidBody.AddForce(force);
            // 초기화
            aimDirection = Vec2(0.0f, 0.0f);
            chargeTime = 0.0f;
        }
    }
private:
    RigidBody& rigidBody;

    Vec2 aimDirection = Vec2(0.0f, 0.0f);
    float chargeTime = 0.0f;
    float maxChargeTime = 2.0f; // 최대 충전 시간
    float forceFactor = 80000.0f; // 힘의 계수

	Vec2 cameraOffset = Vec2(0.0f, 3.0f);
};

static void glfwErrorCallback(int error, const char* description)
{
        printf("GLFW error %d: %s\n", error, description);
}

static void Reshape(GLFWwindow*, int w, int h)
{
        width = w;
        height = h > 0 ? h : 1;

        if (gCameraSystem != nullptr)
        {
                gCameraSystem->OnResize(width, height);
                gCameraSystem->ApplyView();
        }
}

GameObject* createPlayer(GameLoop& gameLoop, float startX, float startY, float mass, physics::Body::ShapeType shapeType)
{
    auto playerObject = std::make_unique<GameObject>();
    auto playerBody = std::make_unique<RigidBody>(
        playerObject.get(),
        Vec2(kCircleRadius * 2.0f, kCircleRadius * 2.0f),
        mass,
        shapeType,
        Vec2(startX, startY));
    auto playerController = std::make_unique<PlayerController>(playerObject.get(), *playerBody);
    auto playerRenderer = std::make_unique<TextureRenderer>(playerObject.get(), *playerBody, playerTextureFile);
    playerObject->AddComponent(playerBody.get());
    playerObject->AddComponent(playerRenderer.get());
    playerObject->AddComponent(playerController.get());
    if (!gameLoop.AddGameObject(playerObject.get()))
    {
        fprintf(stderr, "Failed to register player object with the game loop.\n");
        return nullptr;
    }
    return playerObject.release();
}

GameObject* createBox(GameLoop& gameLoop, float startX, float startY, float mass)
{
    auto boxObject = std::make_unique<GameObject>();
    auto boxBody = std::make_unique<RigidBody>(
        boxObject.get(),
        Vec2(kBoxSize, kBoxSize),
        mass,
        physics::Body::ShapeType::Box,
        Vec2(startX, startY));
    auto boxRenderer = std::make_unique<BodyRenderer>(boxObject.get(), *boxBody);
    boxObject->AddComponent(boxBody.get());
    boxObject->AddComponent(boxRenderer.get());
    if (!gameLoop.AddGameObject(boxObject.get()))
    {
        fprintf(stderr, "Failed to register box object with the game loop.\n");
        return nullptr;
    }
    return boxObject.release();
}

//GameObject* createJoint(GameLoop& gameLoop, RigidBody* bodyA, RigidBody* bodyB, Vec2 anchorA, Vec2 anchorB)
//{
//    auto jointObject = std::make_unique<GameObject>();
//    auto jointComponent = std::make_unique<JointComponent>(
//        jointObject.get(),
//        bodyA,
//        bodyB,
//        anchorA,
//        anchorB);
//    jointObject->AddComponent(jointComponent.get());
//    if (!gameLoop.AddGameObject(jointObject.get()))
//    {
//        fprintf(stderr, "Failed to register joint object with the game loop.\n");
//        return nullptr;
//    }
//    return jointObject.release();
//}

GameObject* createInvironmentBox(GameLoop& gameLoop, float posX, float posY, float sizeX, float sizeY)
{
    auto boxObject = std::make_unique<GameObject>();
    auto boxBody = std::make_unique<RigidBody>(
        boxObject.get(),
        Vec2(sizeX, sizeY),
        FLT_MAX,
        physics::Body::ShapeType::Box,
        Vec2(posX, posY));
    auto boxRenderer = std::make_unique<BodyRenderer>(boxObject.get(), *boxBody);
    boxObject->AddComponent(boxBody.get());
    boxObject->AddComponent(boxRenderer.get());
    if (!gameLoop.AddGameObject(boxObject.get()))
    {
        fprintf(stderr, "Failed to register environment box object with the game loop.\n");
        return nullptr;
    }
    return boxObject.release();
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

        CameraSystem cameraSystem;
        if (!cameraSystem.Initialize(mainWindow))
        {
                fprintf(stderr, "Failed to initialize camera system.\n");
                glfwTerminate();
                return -1;
        }

        int currentWidth = 0;
        int currentHeight = 0;
        glfwGetWindowSize(mainWindow, &currentWidth, &currentHeight);
        width = currentWidth;
        height = currentHeight > 0 ? currentHeight : 1;

        gCameraSystem = &cameraSystem;

        glfwSetWindowSizeCallback(mainWindow, Reshape);
        Reshape(mainWindow, width, height);

        FrameRenderer frameRenderer(mainWindow);
        PhysicsManager physicsManager(Vec2(0.0f, -10.0f), 10);
        InputSystem inputSystem;
        if (!inputSystem.Initialize(mainWindow))
        {
                fprintf(stderr, "Failed to initialize input system.\n");
                glfwTerminate();
                return -1;
        }

        GameLoop gameLoop;
        gameLoop.SetPhysicsManager(&physicsManager);
        gameLoop.SetRenderer(&frameRenderer);
        gameLoop.SetInputSystem(&inputSystem);
        gameLoop.SetCameraSystem(&cameraSystem);

#pragma region Background

		// 배경 오브젝트
		auto backgroundObject = std::make_unique<GameObject>();
        auto backgroundBody = std::make_unique<RigidBody>(backgroundObject.get(),
            backgroundSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
			backgroundPosition);
        auto backgroundRenderer = std::make_unique<TextureRenderer>(backgroundObject.get(), *backgroundBody, backgroundTextureFile);

		backgroundBody->SetActive(false); // 렌더링을 위해 추가한 바디는 물리 연산에서 제외

		// 바디 180도 회전
		backgroundBody->GetBody()->rotation = 3.14159f; // 라디안 단위

        backgroundObject->AddComponent(backgroundBody.get());
        backgroundObject->AddComponent(backgroundRenderer.get());
        if (!gameLoop.AddGameObject(backgroundObject.get()))
        {
                fprintf(stderr, "Failed to register background object with the game loop.\n");
                glfwTerminate();
                return -1;
		}

#pragma endregion

#pragma region Ground

        auto groundObject = std::make_unique<GameObject>();
        auto groundBody = std::make_unique<RigidBody>(
            groundObject.get(),
            groundSize,
            FLT_MAX,
			physics::Body::ShapeType::Box,
			groundPosition);
        auto groundRenderer = std::make_unique<TextureRenderer>(groundObject.get(), *groundBody, groundTextureFile);
        groundObject->AddComponent(groundBody.get());
        groundObject->AddComponent(groundRenderer.get());

        if (!gameLoop.AddGameObject(groundObject.get()))
        {
                fprintf(stderr, "Failed to register ground object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

#pragma endregion

#pragma region wall


        // 왼쪽 벽 오브젝트
        auto leftWallObject = std::make_unique<GameObject>();
        auto leftWallBody = std::make_unique<RigidBody>(
            leftWallObject.get(),
            wallSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
            leftWallPosition);
		auto leftWallRenderer = std::make_unique<TextureRenderer>(leftWallObject.get(), *leftWallBody, invironmentRockTextureFile);
        leftWallObject->AddComponent(leftWallBody.get());
        leftWallObject->AddComponent(leftWallRenderer.get());
        if (!gameLoop.AddGameObject(leftWallObject.get()))
        {
                fprintf(stderr, "Failed to register left wall object with the game loop.\n");
                glfwTerminate();
                return -1;
        }
        // 오른쪽 벽 오브젝트
        auto rightWallObject = std::make_unique<GameObject>();
        auto rightWallBody = std::make_unique<RigidBody>(
            rightWallObject.get(),
            wallSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
            rightWallPosition);
		auto rightWallRenderer = std::make_unique<TextureRenderer>(rightWallObject.get(), *rightWallBody, invironmentRockTextureFile);
        rightWallObject->AddComponent(rightWallBody.get());
        rightWallObject->AddComponent(rightWallRenderer.get());
        if (!gameLoop.AddGameObject(rightWallObject.get()))
        {
                fprintf(stderr, "Failed to register right wall object with the game loop.\n");
                glfwTerminate();
                return -1;
		}

#pragma endregion

#pragma region Invironments

        //왼쪽 바위 위치
		auto rockObject = std::make_unique<GameObject>();
        auto rockBody = std::make_unique<RigidBody>(
            rockObject.get(),
            rockSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
			Vec2(-5.0f, 5.0f));
        auto rockRenderer = std::make_unique<TextureRenderer>(rockObject.get(), *rockBody, invironmentRockTextureFile);
        rockObject->AddComponent(rockBody.get());
        rockObject->AddComponent(rockRenderer.get());
        if (!gameLoop.AddGameObject(rockObject.get()))
        {
                fprintf(stderr, "Failed to register rock object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        //오른쪽 바위 위치: 
        auto rockObject2 = std::make_unique<GameObject>();
        auto rockBody2 = std::make_unique<RigidBody>(
            rockObject2.get(),
            rockSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
			Vec2(1.0f, 8.0f));
        auto rockRenderer2 = std::make_unique<TextureRenderer>(rockObject2.get(), *rockBody2, invironmentRockTextureFile);
        rockObject2->AddComponent(rockBody2.get());
        rockObject2->AddComponent(rockRenderer2.get());
        if (!gameLoop.AddGameObject(rockObject2.get()))
        {
                fprintf(stderr, "Failed to register rock object with the game loop.\n");
                glfwTerminate();
                return -1;
        }

        //왼쪽 바위 위치: -27, 11
		auto rockObject3 = std::make_unique<GameObject>();
        auto rockBody3 = std::make_unique<RigidBody>(
            rockObject3.get(),
            rockSize,
            FLT_MAX,
            physics::Body::ShapeType::Box,
            Vec2(6.0f, 11.0f));
        auto rockRenderer3 = std::make_unique<TextureRenderer>(rockObject3.get(), *rockBody3, invironmentRockTextureFile);
        rockObject3->AddComponent(rockBody3.get());
        rockObject3->AddComponent(rockRenderer3.get());
        if (!gameLoop.AddGameObject(rockObject3.get()))
        {
                fprintf(stderr, "Failed to register rock object with the game loop.\n");
                glfwTerminate();
                return -1;
		}
        

#pragma endregion


#pragma region GameObjects


        // 플레이어 오브젝트
		float playerStartX = 0.0f;
		float playerStartY = 5.0f;
		float playerMass = 50.0f;
		auto playerShapeType = physics::Body::ShapeType::Circle;

		auto playerObject = std::make_unique<GameObject>();
        auto playerBody = std::make_unique<RigidBody>(
            playerObject.get(),
            Vec2(kCircleRadius * 2.0f, kCircleRadius * 2.0f),
            playerMass,
            playerShapeType,
			Vec2(playerStartX, playerStartY));

		//playerBody->SetFriction(0.5f);
		//float oldI = playerBody->GetBody()->I;
		//playerBody->SetInertia(oldI * 100.0f); // 관성 모멘트 증가로 회전 저항 증가

		auto playerController = std::make_unique<PlayerController>(playerObject.get(), *playerBody);
        auto playerRenderer = std::make_unique<TextureRenderer>(playerObject.get(), *playerBody, playerTextureFile);

		playerObject->AddComponent(playerBody.get());
		playerObject->AddComponent(playerRenderer.get());
		playerObject->AddComponent(playerController.get());
        


		if (!gameLoop.AddGameObject(playerObject.get()))
		{
			fprintf(stderr, "Failed to register player object with the game loop.\n");
			glfwTerminate();
			return -1;
		}

        // 플레이어에 연결된 박스 1번
		float box1StartX = playerStartX + 3.0f;
		float box1StartY = playerStartY + 0.0f;
		float box1Mass = 3.0f;
		auto box1Object = std::make_unique<GameObject>();
		auto box1Body = std::make_unique<RigidBody>(
			box1Object.get(),
			Vec2(kBoxSize, kBoxSize),
			box1Mass,
			physics::Body::ShapeType::Box,
			Vec2(box1StartX, box1StartY));
		auto box1Renderer = std::make_unique<BodyRenderer>(box1Object.get(), *box1Body);
		box1Object->AddComponent(box1Body.get());
		box1Object->AddComponent(box1Renderer.get());
		if (!gameLoop.AddGameObject(box1Object.get()))
		{
			fprintf(stderr, "Failed to register box object with the game loop.\n");
			glfwTerminate();
			return -1;
		}

		// 플레이어와 박스 사이에 조인트 생성
        auto jointObject1 = std::make_unique<GameObject>();
        auto jointComponent1 = std::make_unique<JointComponent>(
            jointObject1.get(),
            playerBody.get(),
            box1Body.get(),
            Vec2(playerStartX , playerStartY));// Vec2((playerStartX + box1StartX) * 0.5, playerStartY));
        jointObject1->AddComponent(jointComponent1.get());
        if (!gameLoop.AddGameObject(jointObject1.get()))
        {
            fprintf(stderr, "Failed to register joint object with the game loop.\n");
            glfwTerminate();
            return -1;
        }
        auto jointRenderer = std::make_unique<JointRenderer>(jointObject1.get(), *jointComponent1);
        jointObject1->AddComponent(jointRenderer.get());
		
		// 박스 1번에 연결된 박스 2번
		float box2StartX = box1StartX + 1.5f;
		float box2StartY = box1StartY + 0.0f;
		float box2Mass = 3.0f;
		auto box2Object = std::make_unique<GameObject>();
		auto box2Body = std::make_unique<RigidBody>(
			box2Object.get(),
			Vec2(kBoxSize, kBoxSize),
			box2Mass,
			physics::Body::ShapeType::Box,
			Vec2(box2StartX, box2StartY));
		auto box2Renderer = std::make_unique<BodyRenderer>(box2Object.get(), *box2Body);
		box2Object->AddComponent(box2Body.get());
		box2Object->AddComponent(box2Renderer.get());
		if (!gameLoop.AddGameObject(box2Object.get()))
		{
			fprintf(stderr, "Failed to register box object with the game loop.\n");
			glfwTerminate();
			return -1;
		}
		// 박스 1번과 박스 2번 사이에 조인트 생성
		auto jointObject2 = std::make_unique<GameObject>();
		auto jointComponent2 = std::make_unique<JointComponent>(
			jointObject2.get(),
			box1Body.get(),
			box2Body.get(),
            Vec2(box1StartX, box1StartY)); // Vec2((box1StartX + box2StartX) * 0.5, playerStartY));
		jointObject2->AddComponent(jointComponent2.get());
        auto jointRenderer2 = std::make_unique<JointRenderer>(jointObject2.get(), *jointComponent2);
        jointObject2->AddComponent(jointRenderer2.get());
		if (!gameLoop.AddGameObject(jointObject2.get()))
		{
			fprintf(stderr, "Failed to register joint object with the game loop.\n");
			glfwTerminate();
			return -1;
		}

        // 박스 2번에 연결된 박스 3번
        float box3StartX = box2StartX + 1.5f;
        float box3StartY = box2StartY + 0.0f;
        float box3Mass = 3.0f;
        auto box3Object = std::make_unique<GameObject>();
        auto box3Body = std::make_unique<RigidBody>(
            box3Object.get(),
            Vec2(kBoxSize, kBoxSize),
            box3Mass,
            physics::Body::ShapeType::Box,
            Vec2(box3StartX, box3StartY));
        auto box3Renderer = std::make_unique<BodyRenderer>(box3Object.get(), *box3Body);
        box3Object->AddComponent(box3Body.get());
        box3Object->AddComponent(box3Renderer.get());
        if (!gameLoop.AddGameObject(box3Object.get()))
        {
            fprintf(stderr, "Failed to register box object with the game loop.\n");
            glfwTerminate();
            return -1;
        }
        // 박스 1번과 박스 2번 사이에 조인트 생성
        auto jointObject3 = std::make_unique<GameObject>();
        auto jointComponent3 = std::make_unique<JointComponent>(
            jointObject3.get(),
            box2Body.get(),
            box3Body.get(),
            Vec2(box2StartX, box2StartY)); // Vec2((box1StartX + box2StartX) * 0.5, playerStartY));
        jointObject3->AddComponent(jointComponent3.get());
        auto jointRenderer3 = std::make_unique<JointRenderer>(jointObject3.get(), *jointComponent3);
        jointObject3->AddComponent(jointRenderer3.get());
        if (!gameLoop.AddGameObject(jointObject3.get()))
        {
            fprintf(stderr, "Failed to register joint object with the game loop.\n");
            glfwTerminate();
            return -1;
        }



#pragma endregion   // GameObjects



        gameLoop.Run();

        glfwTerminate();
        return 0;
}
