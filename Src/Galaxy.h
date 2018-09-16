#define		SCREEN_WIDTH		800
#define		SCREEN_HEIGHT		600

float dt = 1.0f / 60.0f;
size_t lastTime, newTime;
float accTime, frameTime;
	
bool keymap[256];

struct RENDER_PARAMS {
	bool tree;
};

RENDER_PARAMS renderParams;

void initGraphics();
void draw();
void reshape(int, int);
void idle();
void keyboard(unsigned char, int, int);
void keyboardUp(unsigned char, int, int);
void makeScreenShot(int);
bool initApp(int, char**);
void readGlxFile(char*);