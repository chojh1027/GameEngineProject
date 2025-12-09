#include "TextureRenderer.h"

#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>

#include "GLFW/glfw3.h"
#include "box2d-lite/MathUtils.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

#include <zlib.h>

namespace
{
    namespace fs = std::experimental::filesystem;

    struct PngImage
    {
        int width = 0;
        int height = 0;
        std::vector<unsigned char> pixels; // RGBA8
    };

    uint32_t ReadBigEndianUInt32(const unsigned char* data)
    {
        return (static_cast<uint32_t>(data[0]) << 24) |
               (static_cast<uint32_t>(data[1]) << 16) |
               (static_cast<uint32_t>(data[2]) << 8) |
               static_cast<uint32_t>(data[3]);
    }

    bool DecodeScanlines(const std::vector<unsigned char>& compressed,
                         int width,
                         int height,
                         int bytesPerPixel,
                         std::vector<unsigned char>& outPixels)
    {
        uLongf uncompressedSize = static_cast<uLongf>((bytesPerPixel * width + 1) * height);
        std::vector<unsigned char> inflated(uncompressedSize);

        if (uncompress(inflated.data(), &uncompressedSize, compressed.data(), static_cast<uLongf>(compressed.size())) != Z_OK)
            return false;

        const int stride = bytesPerPixel * width;
        outPixels.resize(static_cast<size_t>(stride * height));

        auto PaethPredictor = [](int a, int b, int c) {
            int p = a + b - c;
            int pa = std::abs(p - a);
            int pb = std::abs(p - b);
            int pc = std::abs(p - c);
            if (pa <= pb && pa <= pc)
                return a;
            if (pb <= pc)
                return b;
            return c;
        };

        for (int y = 0; y < height; ++y)
        {
            const unsigned char* scanline = inflated.data() + y * (stride + 1);
            unsigned char filter = scanline[0];
            const unsigned char* src = scanline + 1;
            unsigned char* dst = outPixels.data() + y * stride;

            switch (filter)
            {
            case 0: // None
                std::copy(src, src + stride, dst);
                break;
            case 1: // Sub
                for (int x = 0; x < stride; ++x)
                {
                    int left = (x >= bytesPerPixel) ? dst[x - bytesPerPixel] : 0;
                    dst[x] = static_cast<unsigned char>((src[x] + left) & 0xff);
                }
                break;
            case 2: // Up
                for (int x = 0; x < stride; ++x)
                {
                    int up = (y > 0) ? outPixels[(y - 1) * stride + x] : 0;
                    dst[x] = static_cast<unsigned char>((src[x] + up) & 0xff);
                }
                break;
            case 3: // Average
                for (int x = 0; x < stride; ++x)
                {
                    int left = (x >= bytesPerPixel) ? dst[x - bytesPerPixel] : 0;
                    int up = (y > 0) ? outPixels[(y - 1) * stride + x] : 0;
                    int avg = (left + up) / 2;
                    dst[x] = static_cast<unsigned char>((src[x] + avg) & 0xff);
                }
                break;
            case 4: // Paeth
                for (int x = 0; x < stride; ++x)
                {
                    int left = (x >= bytesPerPixel) ? dst[x - bytesPerPixel] : 0;
                    int up = (y > 0) ? outPixels[(y - 1) * stride + x] : 0;
                    int upLeft = (y > 0 && x >= bytesPerPixel) ? outPixels[(y - 1) * stride + x - bytesPerPixel] : 0;
                    int predictor = PaethPredictor(left, up, upLeft);
                    dst[x] = static_cast<unsigned char>((src[x] + predictor) & 0xff);
                }
                break;
            default:
                return false;
            }
        }

        return true;
    }

    bool LoadPngFile(const fs::path& path, PngImage& outImage)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (data.size() < 8)
            return false;

        const unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        if (!std::equal(std::begin(signature), std::end(signature), data.begin()))
            return false;

        size_t offset = 8;
        std::vector<unsigned char> idat;
        uint32_t width = 0;
        uint32_t height = 0;
        int bitDepth = 0;
        int colorType = 0;
        int interlace = 0;

        while (offset + 8 <= data.size())
        {
            uint32_t length = ReadBigEndianUInt32(&data[offset]);
            offset += 4;
            if (offset + 4 > data.size())
                return false;
            std::array<char, 5> chunkType = {0, 0, 0, 0, 0};
            for (int i = 0; i < 4; ++i)
                chunkType[i] = static_cast<char>(data[offset + i]);
            offset += 4;

            if (offset + length + 4 > data.size())
                return false;

            const unsigned char* chunkData = data.data() + offset;
            if (chunkType[0] == 'I' && chunkType[1] == 'H' && chunkType[2] == 'D' && chunkType[3] == 'R')
            {
                if (length < 13)
                    return false;
                width = ReadBigEndianUInt32(chunkData);
                height = ReadBigEndianUInt32(chunkData + 4);
                bitDepth = chunkData[8];
                colorType = chunkData[9];
                interlace = chunkData[12];
            }
            else if (chunkType[0] == 'I' && chunkType[1] == 'D' && chunkType[2] == 'A' && chunkType[3] == 'T')
            {
                idat.insert(idat.end(), chunkData, chunkData + length);
            }
            else if (chunkType[0] == 'I' && chunkType[1] == 'E' && chunkType[2] == 'N' && chunkType[3] == 'D')
            {
                break;
            }

            offset += length + 4; // skip CRC
        }

        if (width == 0 || height == 0 || idat.empty())
            return false;

        if (interlace != 0)
            return false; // only support no interlace

        if (!(bitDepth == 8 && (colorType == 2 || colorType == 6)))
            return false; // only support RGB/RGBA 8-bit

        const int bytesPerPixel = (colorType == 6) ? 4 : 3;
        std::vector<unsigned char> raw;
        if (!DecodeScanlines(idat, static_cast<int>(width), static_cast<int>(height), bytesPerPixel, raw))
            return false;

        outImage.width = static_cast<int>(width);
        outImage.height = static_cast<int>(height);
        outImage.pixels.resize(static_cast<size_t>(width * height * 4));

        for (int i = 0; i < static_cast<int>(width * height); ++i)
        {
            const unsigned char* src = raw.data() + i * bytesPerPixel;
            unsigned char* dst = outImage.pixels.data() + i * 4;
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = (bytesPerPixel == 4) ? src[3] : 255;
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

    if (LoadTextureFromPng(texturePath))
        return true;

    return false;
}

bool TextureRenderer::LoadTextureFromPng(const std::experimental::filesystem::path& texturePath)
{
    PngImage image;
    if (!LoadPngFile(texturePath, image))
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
