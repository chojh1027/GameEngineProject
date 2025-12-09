#pragma once

#include <experimental/filesystem>
#include <string>
#include <vector>

#include "glad/glad.h"

#include "my_engine/Component.h"
#include "my_engine/physics/RigidBody.h"

class TextureRenderer : public Component
{
public:
    TextureRenderer(GameObject* owner, const RigidBody& rigidBodyRef, std::string textureFileName);

    void Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    bool LoadTexture();
    bool LoadTextureFromPng(const std::experimental::filesystem::path& texturePath);
    void CreateFallbackTexture();

    const RigidBody& rigidBody;
    std::string textureFile;
    GLuint textureId = 0;
    int imageWidth = 0;
    int imageHeight = 0;
};
