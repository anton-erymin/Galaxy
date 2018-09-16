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

	// �������� ������ ��������
    tex->width  = ilGetInteger(IL_IMAGE_WIDTH);     
    tex->height = ilGetInteger(IL_IMAGE_HEIGHT);   
    tex->bpp		= ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
 
    // ��������� ������ � ���� ��������
    tex->imageData = ilGetData();
 
    ilEnable(IL_CONV_PAL);

    // ��� ������ �����������
    unsigned int type = ilGetInteger(IL_IMAGE_FORMAT);
 
    // ���������� ��������
	glGenTextures(1, &tex->texID);

    // ����������� ������ �������� � ID
    glBindTexture(GL_TEXTURE_2D, tex->texID);

	//glTexImage2D(GL_TEXTURE_2D, 0, 3, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->imageData);
    // ������ ���-����
    gluBuild2DMipmaps(GL_TEXTURE_2D, tex->bpp, tex->width, tex->height, type, GL_UNSIGNED_BYTE, tex->imageData);

    // ������������� �������� �������
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}