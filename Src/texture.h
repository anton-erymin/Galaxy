#pragma once

#include <cstdint>

#include "gl\glut.h"

typedef struct TextureImage {
    unsigned char *imageData;

    int	 bpp;
    int	 width;
    int	 height;

    uint32_t texID;
} TextureImage;

class TextureLoader {
public:
    void load(const char *fname, TextureImage &tex);
    void free(TextureImage &tex);
};

