#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include "gl\freeglut.h"

#include "Galaxy.h"
#include "GalaxySystem.h"
#include "constants.h"
#include "Universe.h"
#include "texture.h"

Universe universe{ UNIVERSE_SIZE };

extern TextureImage starTexture;

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 80.0f;

float DT;
float universeSize;

bool started = false;
bool saveToFiles;
int mode;
int num1, num2;

extern int curLayer;

int main(int argc, char** argv) {
    if (!initApp(argc, argv)) {
        return 0;
    }

    glutInit(&argc, argv);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(300, 150);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Galaxy Model 1.1");

    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);

    initGraphics();

    glutMainLoop();

    return 0;
}


bool initApp(int argc, char* argv[]) {
    printf("Galaxy Model 1.1\nCopyright (c) Laxe Studio 2014\n\n");


    DT = 0.1f;
    universeSize = UNIVERSE_SIZE;
    saveToFiles = false;

    //if (argc > 0)
    //{
    //	//readGlxFile(argv[1]);
    //	readGlxFile("one galaxy.glx");
    //}
    //else
    {
        printf("TO CHOOSE THE DEFAULT VALUE JUST PRESS ENTER\n\n");

        char c;

        // Кол-во частиц в балдже
        char buf[64];
        printf("Number of stars in bulge       (default is 2000): ");
        gets_s(buf);
        if (strlen(buf) == 0)
            num1 = GLX_BULGE_NUM;
        else
            num1 = atoi(buf);

        // Кол-во частиц в диске
        printf("Number of stars in disk        (default is 10000): ");
        gets_s(buf);
        if (strlen(buf) == 0)
            num2 = GLX_DISK_NUM;
        else
            num2 = atoi(buf);

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

        universe.addGalaxy(GalaxySystem{ lpVec3{0.0f, 0.0f, 0.0f},
            num1,
            num2,
            GLX_BULGE_RADIUS,
            GLX_DISK_RADIUS,
            GLX_HALO_RADIUS,
            GLX_DISK_THICKNESS,
            GLX_BULGE_MASS,
            GLX_HALO_MASS,
            GLX_STAR_MASS,
            true });

        /*float d = 1.0f;
        universe->galaxies_[1] = new GalaxySystem(lpVec3(160.0f, 0.0f, 0.0f),
                                                   num1 / d,
                                                   num2 / d,
                                                   num3 / d,
                                                   mode,
                                                   GLX_BULGE_RADIUS / d,
                                                   GLX_DISK_RADIUS / d,
                                                   GLX_GALO_RADIUS / d,
                                                   GLX_THICKNESS / d,
                                                   GLX_BULGE_MASS / d,
                                                   GLX_STAR_MASS / d,
                                                   GLX_DISK_MASS_RATIO,
                                                   true);*/

                                                   //universe->galaxies_[1]->init();

                                                   //universe->galaxies_[1]->particles_[0]->m_linearVel.setTo(-0.19f, 0.0f, -0.3f);

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


    memset(keymap, 0, 256 * sizeof(bool));

    //started = true;

    return true;
}


void initGraphics() {
    glClearColor(0.0f, 0.0f, 0.00f, 0.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    TextureLoader starLoader;
    starLoader.load("star edited.png", starTexture);

    renderParams.tree = false;
}



void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    //gluLookAt(cameraX, cameraY, cameraZ, cameraX, cameraY, 0.0f, 0.0f, 1.0f, 0.0f);

    gluLookAt(cameraX, cameraY, cameraZ, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);


    int mode = GLX_RENDER_DISK | (renderParams.tree ? GLX_RENDER_TREE : 0);
    universe.draw(mode);


    glutSwapBuffers();


    static int imageId = 0;
    if (started && saveToFiles)
        makeScreenShot(imageId++);
}


void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / (float)height, 1.0f, 100000000.0f);
    glMatrixMode(GL_MODELVIEW);
}


void idle() {
    if (started)
        universe.step(DT);



    static int lastUpdate = 0;
    static int frames = 0;
    char buf[256];
    int currentTime = GetTickCount();
    frames++;

    if (currentTime - lastUpdate >= 1000) {
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



void makeScreenShot(int id) {
    // Массив данных будущего изображения
    static unsigned char *output = 0;

    if (!output) {
        // Выделяем необходимую память: ширина*высота*3 цветовых бита
        output = new unsigned char[3 * SCREEN_WIDTH * SCREEN_HEIGHT];
    }

    // Для получения данных экрана используем функцию glReadPixels:
    glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, output);


    FILE *sFile = 0;

    // Обьявляем переменные, которые понадобятся нам в дальнейшем:
    unsigned char tgaHeader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * colorMode; i += colorMode) {
        tempColors = output[i];
        output[i] = output[i + 2];
        output[i + 2] = tempColors;
    }

    // Записываем данные изображения:
    fwrite(output, SCREEN_WIDTH * SCREEN_HEIGHT * colorMode, 1, sFile);

    // Закрываем файл
    fclose(sFile);
}



void keyboard(unsigned char key, int x, int y) {
    keymap[key] = true;

    if (key == '+') cameraZ /= 1.3f;
    if (key == '-') cameraZ *= 1.3f;
    if (key == 't' || key == 'T') renderParams.tree = !renderParams.tree;
    if (key == ']') DT *= 1.2f;
    if (key == '[') DT *= 0.8f;
    if (key == ' ') started = false;
    if (key == 13) {
        if (!started) started = true;
    }

    if (key == 'u') curLayer++;
}


void keyboardUp(unsigned char key, int x, int y) {
    keymap[key] = false;
}




void readGlxFile(char* fname) {
    FILE *f = fopen(fname, "r");
    if (f == NULL) {
        perror("Error opening file");
        exit(1);
    }


    char line[256];

    GalaxySystem *glx;

    while (feof(f) == 0) {
        fgets(line, 255, f);

        if (line[0] == '#')
            continue;


        if (strncmp(line, "[GALAXY]", 8) == 0) {

            glx = new GalaxySystem();
            universe.addGalaxy(*glx);


        }


    }



    fclose(f);
}