#include "TextureRenderer.h"

#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include "GLFW/glfw3.h"
#include "box2d-lite/MathUtils.h"
#include <cmath>
#include <cstdint>
#include <vector>

namespace
{
    namespace fs = std::experimental::filesystem;

    struct BmpImage
    {
        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels; // RGBA8
    };

    template <typename T>
    bool ReadLittleEndian(std::ifstream& file, T& outValue)
    {
        unsigned char bytes[sizeof(T)];
        if (!file.read(reinterpret_cast<char*>(bytes), sizeof(T)))
            return false;

        outValue = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
            outValue |= static_cast<T>(bytes[i]) << (8 * i);
        return true;
    }

    bool LoadBmpFile(const fs::path& path, BmpImage& outImage)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        unsigned char signature[2];
        if (!file.read(reinterpret_cast<char*>(signature), 2))
            return false;
        if (signature[0] != 'B' || signature[1] != 'M')
            return false;

        uint32_t fileSize = 0;
        uint16_t reserved1 = 0;
        uint16_t reserved2 = 0;
        uint32_t pixelOffset = 0;
        if (!ReadLittleEndian(file, fileSize) ||
            !ReadLittleEndian(file, reserved1) ||
            !ReadLittleEndian(file, reserved2) ||
            !ReadLittleEndian(file, pixelOffset))
            return false;

        uint32_t dibHeaderSize = 0;
        if (!ReadLittleEndian(file, dibHeaderSize))
            return false;
        if (dibHeaderSize < 40)
            return false; // require BITMAPINFOHEADER or larger

        int32_t width = 0;
        int32_t height = 0;
        uint16_t planes = 0;
        uint16_t bitsPerPixel = 0;
        uint32_t compression = 0;
        uint32_t imageSize = 0;

        if (!ReadLittleEndian(file, width) ||
            !ReadLittleEndian(file, height) ||
            !ReadLittleEndian(file, planes) ||
            !ReadLittleEndian(file, bitsPerPixel) ||
            !ReadLittleEndian(file, compression) ||
            !ReadLittleEndian(file, imageSize))
            return false;

        if (planes != 1)
            return false;
        if (!(bitsPerPixel == 24 || bitsPerPixel == 32))
            return false; // only support 24-bit BGR or 32-bit BGRA
        if (compression != 0)
            return false; // only support BI_RGB (no compression)

        // Skip the rest of the DIB header if present
        if (dibHeaderSize > 24)
        {
            file.seekg(static_cast<std::streamoff>(dibHeaderSize - 24), std::ios::cur);
            if (!file)
                return false;
        }

        // Jump to pixel data
        file.seekg(static_cast<std::streamoff>(pixelOffset), std::ios::beg);
        if (!file)
            return false;

        const int absHeight = std::abs(height);
        const int rowStrideInFile = ((bitsPerPixel * width + 31) / 32) * 4; // rows aligned to 4 bytes
        const int bytesPerPixel = bitsPerPixel / 8;

        std::vector<unsigned char> rawData(static_cast<size_t>(rowStrideInFile * absHeight));
        if (!file.read(reinterpret_cast<char*>(rawData.data()), rawData.size()))
            return false;

        outImage.width = width;
        outImage.height = absHeight;
        outImage.pixels.resize(static_cast<size_t>(width * absHeight * 4));

        const bool bottomUp = height > 0;
        for (int y = 0; y < absHeight; ++y)
        {
            int srcRow = bottomUp ? (absHeight - 1 - y) : y;
            const unsigned char* src = rawData.data() + srcRow * rowStrideInFile;
            unsigned char* dst = outImage.pixels.data() + static_cast<size_t>(y * width * 4);

            for (int x = 0; x < width; ++x)
            {
                const unsigned char* pixel = src + x * bytesPerPixel;
                dst[x * 4 + 0] = pixel[2];
                dst[x * 4 + 1] = pixel[1];
                dst[x * 4 + 2] = pixel[0];
                dst[x * 4 + 3] = (bytesPerPixel == 4) ? pixel[3] : 255;
            }
        }

        return true;
    }
}

TextureRenderer::TextureRenderer(GameObject* owner, const RigidBody& rigidBodyRef, std::string textureFileName)
    : Component(owner)
    , rigidBody(rigidBodyRef)
    , textureFile(std::move(textureFileName))
{
}

void TextureRenderer::Init()
{
    if (!LoadTexture())
    {
        std::cerr << "Failed to load texture from file. Using fallback pattern.\n";
        CreateFallbackTexture();
    }
}

bool TextureRenderer::LoadTexture()
{
    namespace fs = std::experimental::filesystem;

    fs::path texturePath(textureFile);
    if (!texturePath.has_parent_path())
        texturePath = fs::path("src") / texturePath;

    if (LoadTextureFromBmp(texturePath))
        return true;

    return false;
}

bool TextureRenderer::LoadTextureFromBmp(const std::experimental::filesystem::path& texturePath)
{
    BmpImage image;
    if (!LoadBmpFile(texturePath, image))
    {
        std::cerr << "Could not open or decode texture file: " << texturePath << "\n";
        return false;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 static_cast<GLsizei>(image.width),
                 static_cast<GLsizei>(image.height),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image.pixels.data());

    imageWidth = image.width;
    imageHeight = image.height;

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void TextureRenderer::CreateFallbackTexture()
{
    const int fallbackSize = 2;
    const unsigned char checkerboard[fallbackSize * fallbackSize * 4] = {
        255, 255, 255, 255, 128, 128, 128, 255,
        128, 128, 128, 255, 255, 255, 255, 255,
    };

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 fallbackSize,
                 fallbackSize,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 checkerboard);

    imageWidth = fallbackSize;
    imageHeight = fallbackSize;
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureRenderer::Update(float deltaTime)
{
    (void)deltaTime;

    const physics::Body* body = rigidBody.GetBody();
    if (body == nullptr || textureId == 0)
        return;

    Mat22 rotation(body->rotation);
    Vec2 center = body->position;
    Vec2 halfSize = 0.5f * body->width;

    Vec2 v1 = center + rotation * Vec2(-halfSize.x, -halfSize.y);
    Vec2 v2 = center + rotation * Vec2(halfSize.x, -halfSize.y);
    Vec2 v3 = center + rotation * Vec2(halfSize.x, halfSize.y);
    Vec2 v4 = center + rotation * Vec2(-halfSize.x, halfSize.y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(v1.x, v1.y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(v2.x, v2.y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(v3.x, v3.y);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(v4.x, v4.y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void TextureRenderer::Destroy()
{
    if (textureId != 0)
    {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
}
