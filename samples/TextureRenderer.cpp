#include "TextureRenderer.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include "GLFW/glfw3.h"
#include "box2d-lite/MathUtils.h"
#include "png.h"

#ifdef _MSC_VER
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "zlib.lib")
#endif

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
    std::filesystem::path texturePath(textureFile);
    if (!texturePath.has_parent_path())
        texturePath = std::filesystem::path("src") / texturePath;

    if (LoadTextureFromPng(texturePath))
        return true;

    return false;
}

bool TextureRenderer::LoadTextureFromPng(const std::filesystem::path& texturePath)
{
    FILE* fp = fopen(texturePath.string().c_str(), "rb");
    if (fp == nullptr)
    {
        std::cerr << "Could not open texture file: " << texturePath << "\n";
        return false;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr)
    {
        fclose(fp);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr)
    {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bit_depth = 0;
    int color_type = 0;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if ((color_type & PNG_COLOR_MASK_ALPHA) == 0)
        png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_size_t rowbytes = png_get_rowbytes(png, info);
    std::vector<png_byte> imageData(rowbytes * height);
    std::vector<png_bytep> rowPointers(height);
    for (png_uint_32 y = 0; y < height; ++y)
        rowPointers[y] = imageData.data() + y * rowbytes;

    png_read_image(png, rowPointers.data());
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 static_cast<GLsizei>(width),
                 static_cast<GLsizei>(height),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 imageData.data());

    imageWidth = static_cast<int>(width);
    imageHeight = static_cast<int>(height);

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
