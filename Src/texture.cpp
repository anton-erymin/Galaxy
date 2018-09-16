#include "texture.h"


lTexture::lTexture(void)
{
	ilInit();
	iluInit();
}


lTexture::~lTexture(void)
{
}


void lTexture::load(ILenum fileType, char *fname, TextureImage *tex)
{
	ilLoad(fileType, fname);

	// Получаем данные текстуры
    tex->width  = ilGetInteger(IL_IMAGE_WIDTH);     
    tex->height = ilGetInteger(IL_IMAGE_HEIGHT);   
    tex->bpp		= ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
 
    // Загружаем данные в нашу текстуру
    tex->imageData = ilGetData();
 
    ilEnable(IL_CONV_PAL);

    // Тип данных изображения
    unsigned int type = ilGetInteger(IL_IMAGE_FORMAT);
 
    // Генерируем текстуру
	glGenTextures(1, &tex->texID);

    // Привязываем данные текстуры к ID
    glBindTexture(GL_TEXTURE_2D, tex->texID);

	//glTexImage2D(GL_TEXTURE_2D, 0, 3, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->imageData);
    // биндим мип-мапы
    gluBuild2DMipmaps(GL_TEXTURE_2D, tex->bpp, tex->width, tex->height, type, GL_UNSIGNED_BYTE, tex->imageData);

    // Устанавливаем качество текстур
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}