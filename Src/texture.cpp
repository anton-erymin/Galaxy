#include "texture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void TextureLoader::load(const char *fname, TextureImage &tex) {
    tex.imageData = stbi_load(fname, &tex.width, &tex.height, &tex.bpp, 0);

    if (!tex.imageData) {
        throw std::invalid_argument(fname);
    }

    glGenTextures(1, &tex.texID);
    glBindTexture(GL_TEXTURE_2D, tex.texID);
    //glTexImage2D(GL_TEXTURE_2D, 0, 3, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->imageData);
    gluBuild2DMipmaps(GL_TEXTURE_2D, tex.bpp, tex.width, tex.height, GL_RGB, GL_UNSIGNED_BYTE, tex.imageData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TextureLoader::free(TextureImage &tex) {
    if (tex.imageData) {
        stbi_image_free(tex.imageData);
        tex.imageData = nullptr;
    }
}