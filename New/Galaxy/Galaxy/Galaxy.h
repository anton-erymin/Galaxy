

#define		SCREEN_WIDTH		800
#define		SCREEN_HEIGHT		600

#define		WINDOW_CAPTION		"Galaxy Model 1.1"


DWORD lastTime, newTime;
float accTime, frameTime;
	
bool keymap[256];


struct RENDER_PARAMS
{
	bool tree;
};


struct RENDER_PARAMS renderParams;



bool initConsoleApp(int, char**);
bool readModelFromGlxFile(char*);
void initGLUT(int, char**);
void createDefaultModel();

void initGraphics();
void initGalaxy(int);
void draw();
void reshape(int, int);
void idle();
void keyboard(unsigned char, int, int);
void keyboardUp(unsigned char, int, int);
void itemSelected(int);
void makeScreenShot(int);
void renderUniverse();
