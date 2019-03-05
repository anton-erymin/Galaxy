#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

constexpr uint32_t cInvalidTextureId = static_cast<uint32_t>(-1);

struct Image
{
    std::vector<uint8_t> data;
    uint8_t bytesPerPixel = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t GetTextureId() const;

    void SaveTga(const char* filename) const;

private:
    mutable uint32_t textureId = cInvalidTextureId;
};

class ImageLoader
{
public:
    void Load(const char* imageName, const char *filename);
    const Image& GetImage(const char* imageName) const;
    void GenTextureIds() const;
private:
    std::unordered_map<std::string, Image> images;
};

struct Rect
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

class ScreenShoter
{
public:
    static const Image& GetScreenshot(const Rect& area);

private:
    static Image screenshot;
};