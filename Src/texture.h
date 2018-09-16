#pragma once

#include "gl\glut.h"

#include "IL\il.h"
#include "IL\ilu.h"



typedef struct TextureImage
{
	unsigned char	*imageData;
	unsigned int	 bpp;
	unsigned int	 width;
	unsigned int	 height;
	unsigned int	 texID;
} TextureImage;



class lTexture
{
public:

	lTexture(void);
	~lTexture(void);

	void load(ILenum fileType, char *fname, TextureImage *tex);
	void free(TextureImage *tex);
};

