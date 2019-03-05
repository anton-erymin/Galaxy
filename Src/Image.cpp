#include "Image.h"
#include "Application.h"

#include <stdexcept>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gl\glut.h"

Image ScreenShoter::screenshot;

void ImageLoader::Load(const char* imageName, const char *filename)
{
    assert(imageName && filename);

    if (images.count(imageName) > 0)
    {
        throw std::runtime_error("Image name duplicate");
    }

    int width = 0, height = 0, bpp = 0;
    stbi_uc *data = stbi_load(filename, &width, &height, &bpp, 0);

    if (!data)
    {
        throw std::runtime_error("Can't load image");
    }

    assert(width > 0 && height > 0 && (bpp == 3 || bpp == 4));

    Image& image = images[imageName];

    image.width = width;
    image.height = height;
    image.bytesPerPixel = bpp;
    image.data.resize(width * height * bpp);

    std::memcpy(image.data.data(), data, image.data.size());
    stbi_image_free(data);
}

const Image& ImageLoader::GetImage(const char* imageName) const
{
    assert(imageName);
    return images.at(imageName);
}

void ImageLoader::GenTextureIds() const
{
    for (const auto& image : images)
    {
        image.second.GetTextureId();
    }
}

uint32_t Image::GetTextureId() const
{
    if (textureId == cInvalidTextureId)
    {
        glGenTextures(1, &textureId);
        assert(textureId != cInvalidTextureId);

        GLenum format = bytesPerPixel == 4 ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, format, GL_UNSIGNED_BYTE, data.data());
        gluBuild2DMipmaps(GL_TEXTURE_2D, bytesPerPixel, width, height, format, GL_UNSIGNED_BYTE, data.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return textureId;
}

void Image::SaveTga(const char* filename) const
{
    assert(filename);
    assert(bytesPerPixel == 3);

    FILE *sFile = 0;

    // TGA header
    unsigned char tgaHeader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char header[6];
    unsigned char bits = 0;
    unsigned char tempColors = 0;
    int			  colorMode = 0;

    // Открываем файл скриншота
    sFile = fopen(filename, "wb");

    // Устанавливаем цветовой режим и глубину цвета:
    colorMode = 3;
    bits = 24;

    // Записываем ширину и высоту:
    header[0] = width % 256;
    header[1] = width / 256;
    header[2] = height % 256;
    header[3] = height / 256;
    header[4] = bits;
    header[5] = 0;

    // Записываем хидеры в начало файла:
    fwrite(tgaHeader, sizeof(tgaHeader), 1, sFile);
    fwrite(header, sizeof(header), 1, sFile);

    const uint8_t* pdata = data.data();

    // Поскольку в формате TGA цвета хранятся не в RGB, а в BRG, нам нужно
    // поменять местами наши данные:
    for (int i = 0; i < data.size(); i += colorMode)
    {
        //std::swap(pdata[i], pdata[i + 2]);
    }

    // Записываем данные изображения:
    fwrite(pdata, data.size(), 1, sFile);

    // Закрываем файл
    fclose(sFile);
}

const Image& ScreenShoter::GetScreenshot(const Rect& area)
{
    screenshot.width = area.width;
    screenshot.height = area.height;
    screenshot.bytesPerPixel = 3;

    screenshot.data.resize(3 * area.width * area.height);

    glReadPixels(area.x, area.y, area.width, area.height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data.data());
    return screenshot;
}
