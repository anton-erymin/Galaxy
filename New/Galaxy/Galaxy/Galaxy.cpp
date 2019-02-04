#include <Windows.h>
#include <stdio.h>
#include <gl\freeglut.h>

#include "Galaxy.h"
#include "GalaxySystem.h"
#include "constants.h"
#include "Universe.h"
#include "texture.h"


Universe *universe;

extern TextureImage starTexture;
extern TextureImage dustTexture[3];

float cameraX = 0.0f;
float cameraY = -20.0f;
float cameraZ = 40.0f;


float DT;
float universeSize;


bool started = false;
bool saveToFiles;
int mode;
int num1, num2;

extern int curLayer;


int main(int argc, char* argv[])
{
	if (!initConsoleApp(argc, argv))
		return 0;

	// Инициализируем окно вывода графики
	initGLUT(argc, argv);
	initGraphics();

	glutMainLoop();
	return 0;
}

bool initConsoleApp(int argc, char* argv[])
{
	// Инициализация консольного приложения, чтение файла параметров, настройка параметров моделирования
	printf("Galaxy Model 1.1\nCopyright (c) Laxe Studio 2012-2014\n\n");


	// Значения по умолчанию глобальных параметров
	// Шаг во времени
	DT = 0.1f;
	// Размер области разбиения
	universeSize = UNIVERSE_SIZE;
	// Флаг сохранения кадров (видео) на диск
	saveToFiles = false;

	if (argc > 0)
	{
		// Если программа запущена через файл описания модели то считываем файл
		//readModelFromGlxFile(argv[1]);
		readModelFromGlxFile("D:\\GALAXY\\Galaxy\\Debug\\milkyway.glx");
	}
	else
	{
		printf("TO CHOOSE THE DEFAULT VALUE JUST PRESS ENTER\n\n");

		char c;

		// Будем ли писать кадры на диск?
		printf("Save the frames to the disk    (default is NO) [y/n]?: ");
		c = getchar();
		if (c == 13 || c == 10)
			saveToFiles = false;
		else if (c == 'n' || c == 'N')
			saveToFiles = false;
		else if (c == 'y' || c == 'Y')
			saveToFiles = true;
		else return false;


		// Создаем модель по умолчанию если не задан файл проекта
		createDefaultModel();
	}


	printf("\nControl keys:\n\n");
	printf("ENTER    - Start\n");
	printf("SPACE    - Reset the galaxy\n");
	printf("'+'      - Zoom in\n");
	printf("'-'      - Zoom out\n");
	printf("'WASD'   - Move\n");
	printf("']'      - Speed up\n");
	printf("'['      - Slow down\n");
	printf("'t'      - Toggle quadtree drawing\n");


	universe->init();

	return true;
}


void createDefaultModel()
{
	universe = new Universe(UNIVERSE_SIZE);
	GalaxySystem *glx = new GalaxySystem(lpVec3(0.0f, 0.0f, 0.0f), 
												GLX_BULGE_NUM,
												GLX_DISK_NUM,
												GLX_BULGE_RADIUS,
												GLX_DISK_RADIUS,
												GLX_HALO_RADIUS,
												GLX_DISK_THICKNESS,
												GLX_BULGE_MASS,
												GLX_HALO_MASS,
												GLX_STAR_MASS,
												true);
	universe->addGalaxy(glx);
}


void getLexem(char *line, char *lexem, int &i)
{
	// Пропуск пробелов
	char c = line[i];
	while (c == 32 || c == '\t') c = line[++i];
			
	if (c == 0)
	{
		lexem[0] = 0;
		return;
	}

	// Проверяем разделители
	if (c == '=' || c == ',')
	{
		i++;
		lexem[0] = c;
		lexem[1] = 0;
	}
	else
	{
		// Выделение лексемы
		int j = 0;
		// Считываем буквы, цифры, прочерки, точки и минусы
		while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
			c == '.' || c == '_' || c == '-')
		{
			lexem[j++] = c;
			c = line[++i];
		}
		lexem[j] = 0;
	}
}


bool readModelFromGlxFile(char* fname)
{
	// Открываем файл
	FILE *f = fopen(fname, "r");
	if (f == NULL) 
		perror ("Error opening file");

	char line[256];
	char lexem[64];
	GalaxySystem *glx;
	int i, j;
	char c;
	int linecount = 0;
	float value;

	while (feof(f) == 0)
	{
		fgets(line, 255, f);
		linecount++;

		// Пропускаем коммент и пустые строки
		if (line[0] == '#' || line[0] == 10 || line[0] == 13)
			continue;
		
		if (strncmp(line, "[GALAXY]", 8) == 0)
		{
			if (!universe)
			{
				universe = new Universe(universeSize);
			}

			glx = new GalaxySystem();
			universe->addGalaxy(glx);
		}
		else
		{
			c = 1;
			i = 0;
			while (true)
			{
				// Обработка лексем

				char varname[64];
				getLexem(line, varname, i);
				if (varname[0] == 0)
					break;
	
				getLexem(line, lexem, i);
				if (lexem[0] != '=')
				{
					printf("Строка %d: ожидалось '='.", linecount);
					return false;
				}

				getLexem(line, lexem, i);
				
				value = atof(lexem);
				
				
				if (strcmp(varname, "DT") == 0)
				{
					DT = value;
					break;
				}
				else if (strcmp(varname, "SAVE_FRAMES") == 0)
				{
					saveToFiles = value;
					break;
				}
				else if (strcmp(varname, "UNIVERSE_SIZE") == 0)
				{
					universeSize = value;
					break;
				}
				else if (strcmp(varname, "GLX_BULGE_NUM") == 0)
				{
					glx->m_numBulgeStars = value;
					break;
				}
				else if (strcmp(varname, "GLX_DISK_NUM") == 0)
				{
					glx->m_numDiskStars = value;
					break;
				}
				else if (strcmp(varname, "GLX_DISK_RADIUS") == 0)
				{
					glx->m_diskRadius = value;
					break;
				}
				else if (strcmp(varname, "GLX_BULGE_RADIUS") == 0)
				{
					glx->m_bulgeRadius = value;
					break;
				}
				else if (strcmp(varname, "GLX_HALO_RADIUS") == 0)
				{
					glx->m_haloRadius = value;
					break;
				}
				else if (strcmp(varname, "GLX_DISK_THICKNESS") == 0)
				{
					glx->m_diskThickness = value;
					break;
				}
				else if (strcmp(varname, "GLX_STAR_MASS") == 0)
				{
					glx->m_starMass = value;
					break;
				}
				else if (strcmp(varname, "GLX_BULGE_MASS") == 0)
				{
					glx->m_bulgeMass = value;
					break;
				}
				else if (strcmp(varname, "GLX_HALO_MASS") == 0)
				{
					glx->m_haloMass = value;
					break;
				}
				else break;



			}





		}


	}



	fclose(f);

	return true;
}


void initGLUT(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(300, 150);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	
	glutCreateWindow(WINDOW_CAPTION);

	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	memset(keymap, 0, 256 * sizeof(bool));
}


void initGraphics()
{                             
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glBlendFunc(GL_ONE, GL_ONE);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);

	// Загружаем текстуры
	lTexture *starLoader = new lTexture();
	starLoader->load(IL_PNG, "images\\star.png", &starTexture);
	starLoader->load(IL_PNG, "images\\dust1.png", &dustTexture[0]);
	starLoader->load(IL_PNG, "images\\dust2.png", &dustTexture[1]);
	starLoader->load(IL_PNG, "images\\dust3.png", &dustTexture[2]);
	renderParams.tree = false;
}



void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	glLoadIdentity();
	gluLookAt(cameraX, cameraY, cameraZ, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	int mode = GLX_RENDER_DISK | (renderParams.tree ? GLX_RENDER_TREE : 0);

	renderUniverse();


	glutSwapBuffers();

	static int imageId = 0;
	if (started && saveToFiles)
		makeScreenShot(imageId++);
}


void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)width / (float) height, 1.0f, 100000000.0f);
	glMatrixMode(GL_MODELVIEW);
}


void idle()
{
	if (started)
		universe->step(DT);
	


	static int lastUpdate = 0;
	static int frames = 0;
	char buf[256];
	int currentTime = GetTickCount();
	frames++;

	if (currentTime - lastUpdate >= 1000)
	{
		sprintf(buf, "Galaxy Model [FPS: %d]", frames);
		glutSetWindowTitle(buf);
		frames = 0;
		lastUpdate = currentTime;
	}


	glutPostRedisplay();	



	if (keymap['d'] || keymap['D']) cameraX += cameraZ * 0.015f;
	if (keymap['a'] || keymap['A']) cameraX -= cameraZ * 0.015f;
	if (keymap['w'] || keymap['W']) cameraY += cameraZ * 0.015f;
	if (keymap['s'] || keymap['S']) cameraY -= cameraZ * 0.015f;

}



void makeScreenShot(int id)
{
    // Массив данных будущего изображения
    static unsigned char *output = 0;
	
	if (!output)
	{
		// Выделяем необходимую память: ширина*высота*3 цветовых бита
		output = new unsigned char[3 * SCREEN_WIDTH * SCREEN_HEIGHT];
	}

    // Для получения данных экрана используем функцию glReadPixels:
    glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, output);

    
    FILE *sFile = 0;

    // Обьявляем переменные, которые понадобятся нам в дальнейшем:
    unsigned char tgaHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char header[6];
    unsigned char bits = 0;
    unsigned char tempColors = 0;
	int			  colorMode = 0;

	char fname[64];
	sprintf(fname, "images\\image_%d.tga", id);

    // Открываем файл скриншота
    sFile = fopen(fname, "wb");

    // Устанавливаем цветовой режим и глубину цвета:
    colorMode = 3;
    bits = 24;

    // Записываем ширину и высоту:
    header[0] = SCREEN_WIDTH % 256;
    header[1] = SCREEN_WIDTH / 256;
    header[2] = SCREEN_HEIGHT % 256;
    header[3] = SCREEN_HEIGHT / 256;
    header[4] = bits;
    header[5] = 0;
    
    // Записываем хидеры в начало файла:
    fwrite(tgaHeader, sizeof(tgaHeader), 1, sFile);
    fwrite(header, sizeof(header), 1, sFile);

    // Поскольку в формате TGA цвета хранятся не в RGB, а в BRG, нам нужно
    // поменять местами наши данные:
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * colorMode; i += colorMode)
    {
        tempColors		= output[i    ];
        output[i    ]	= output[i + 2];
        output[i + 2]	= tempColors;
    }

    // Записываем данные изображения:
    fwrite(output, SCREEN_WIDTH * SCREEN_HEIGHT * colorMode, 1, sFile);

    // Закрываем файл
    fclose(sFile);
}



void keyboard(unsigned char key, int x, int y)
{
	keymap[key] = true;

	if (key == '+') cameraZ /= 1.3f;
	if (key == '-') cameraZ *= 1.3f;
	if (key == 't' || key == 'T') renderParams.tree = !renderParams.tree;
	if (key == ']') DT *= 1.2f;
	if (key == '[') DT *= 0.8f;
	if (key == ' ') started = false;
	if (key == 13)
	{
		if (!started) started = true;
	}

	if (key == 'u') curLayer++;
}


void keyboardUp(unsigned char key, int x, int y)
{
	keymap[key] = false;	
}




void renderUniverse()
{
	float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
	lpVec3 v1 = lpVec3(modelview[0], modelview[4], modelview[8]);
	lpVec3 v2 = lpVec3(modelview[1], modelview[5], modelview[9]);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(false);
	

	

	for (int i = 0; i < universe->m_numGalaxies; i++)
	{
		// Отрисовка галактик

		GalaxySystem *glx = universe->m_galaxies[i];
		lpVec3 p1, p2, p3, p4;

		for (int j = 0; j < glx->m_numAll; j++)
		{
			GalaxyParticle *p = glx->m_particles[j];
			if (!p->m_active) 
				continue;

			lpVec3 pos = p->m_pos;
			float s = 0.5f * p->m_size;
			

			p1 = pos - v1 * s - v2 * s;
			p2 = pos - v1 * s + v2 * s;
			p3 = pos + v1 * s + v2 * s;
			p4 = pos + v1 * s - v2 * s;

			float mag = p->m_mag;
			// Квадрат расстояние до частицы от наблюдателя
			float dist = (cameraX - pos.m_x) * (cameraX - pos.m_x) + (cameraY - pos.m_y) * (cameraY - pos.m_y) + (cameraZ - pos.m_z) * (cameraZ - pos.m_z);
			//dist = sqrt(dist);
			if (dist > 5.0f) dist = 5.0f;
			mag /= (dist/ 2);

			
			glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D, p->m_texID);
			glColor3f(p->m_cR * mag, p->m_cG * mag, p->m_cB * mag);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(p1.m_x, p1.m_y, p1.m_z);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(p2.m_x, p2.m_y, p2.m_z);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(p3.m_x, p3.m_y, p3.m_z);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(p4.m_x, p4.m_y, p4.m_z);

			if (p->m_doubleDrawing)
			{
				glTexCoord2f(0.0f, 1.0f); glVertex3fv(&p1.m_x);
				glTexCoord2f(0.0f, 0.0f); glVertex3fv(&p2.m_x);
				glTexCoord2f(1.0f, 0.0f); glVertex3fv(&p3.m_x);
				glTexCoord2f(1.0f, 1.0f); glVertex3fv(&p4.m_x);
			}
			glEnd();

			glPopMatrix();
		}

		
		glDepthMask(true);


		//m_darkMatter.plotPotential();
	}

	//if (mode & GLX_RENDER_TREE) drawTree(m_bht);
}