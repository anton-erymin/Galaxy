#include "Application.h"

#include <iostream>
#include <functional>
#include <chrono>

#include <Windows.h>
#include <stdio.h>

#include "Galaxy.h"
#include "Constants.h"
#include "Image.h"
#include "Solver.h"
#include "Threading.h"

constexpr uint32_t cWindowWidth = 1400;
constexpr uint32_t cWindowHeight = 800;

constexpr char* cWindowCaption = "Galaxy Model 1.1";

extern int curLayer;

Application* Application::instance = nullptr;

Application application;

Application::Application()
{
    assert(!instance);
    instance = this;
}

Application::~Application()
{
    ThreadPool::Destroy();
}

static Universe* CreateDefaultUniverse()
{
    Universe* universe = new Universe(UNIVERSE_SIZE);
    universe->CreateGalaxy();
    return universe;
}

int Application::Run(int argc, char **argv)
{
    ThreadPool::Create(std::thread::hardware_concurrency());
    
    std::cout << "Galaxy Model 0.1\nCopyright (c) Laxe Studio 2012-2014" << std::endl << std::endl;

    imageLoader = std::make_unique<ImageLoader>();

    // Загружаем текстуры
    GetImageLoader().Load("Star", "Data/star.png");
    GetImageLoader().Load("Dust1", "Data/dust1.png");
    GetImageLoader().Load("Dust2", "Data/dust2.png");
    GetImageLoader().Load("Dust3", "Data/dust3.png");

    // Значения по умолчанию глобальных параметров
    // Шаг во времени
    deltaTime = 0.05f;
    // Размер области разбиения
    universeSize = UNIVERSE_SIZE;
    // Флаг сохранения кадров (видео) на диск
    saveToFiles = false;

    cSecsInTimeUnit = std::sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG));
    cMillionYearsInTimeUnit = cSecsInTimeUnit / 3600 / 24 / 365 / 1e+6;

    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    if (argc > 1)
    {
        // Если программа запущена через файл описания модели то считываем файл
        //readModelFromGlxFile(argv[1]);
    }
    else
    {
        printf("TO CHOOSE THE DEFAULT VALUE JUST PRESS ENTER\n\n");

        char c;

        // Будем ли писать кадры на диск?
        printf("Save the frames to the disk    (default is NO) [y/n]?: ");
        //c = getchar();
        c = 13;
        if (c == 13 || c == 10)
            saveToFiles = false;
        else if (c == 'n' || c == 'N')
            saveToFiles = false;
        else if (c == 'y' || c == 'Y')
            saveToFiles = true;
        else return false;


        // Создаем модель по умолчанию если не задан файл проекта
        universe.reset(CreateDefaultUniverse());
    }

    printf("\nControl keys:\n\n");
    printf("ENTER    - Start\n");
    printf("SPACE    - Reset the galaxy\n");
    printf("']'      - Speed up\n");
    printf("'['      - Slow down\n");
    printf("'t'      - Toggle quadtree drawing\n");
    printf("'m'      - Toggle draw mode");

    renderParams.renderTree = true;

    width = cWindowWidth;
    height = cWindowHeight;

    glutInit(&argc, argv);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(300, 150);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow(cWindowCaption);

    glutDisplayFunc([]() { Application::GetInstance().OnDraw(); });
    glutReshapeFunc([](int width, int height) { Application::GetInstance().OnResize(width, height); });
    glutIdleFunc([]() { Application::GetInstance().OnIdle(); });
    glutKeyboardFunc([](unsigned char key, int x, int y) { Application::GetInstance().OnKeyboard(key, x, y); });
    glutKeyboardUpFunc([](unsigned char key, int x, int y) { Application::GetInstance().OnKeyboardUp(key, x, y); });
    glutMouseFunc([](int button, int state, int x, int y) { Application::GetInstance().OnMousePressed(button, state, x, y); });
    glutMotionFunc([](int x, int y) { Application::GetInstance().OnMouseMove(x, y); });
    glutMouseWheelFunc([](int button, int dir, int x, int y) { Application::GetInstance().OnMouseWheel(button, dir, x, y); });

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBlendFunc(GL_ONE, GL_ONE);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_ALPHA_TEST);

    solverBruteforce = std::make_unique<BruteforceSolver>();
    solverBarneshut = std::make_unique<BarnesHutSolver>();

    solver = &*solverBarneshut;

    GetImageLoader().GenTextureIds();

    inputMappings['a'] = &inputState.brightnessUp;
    inputMappings['z'] = &inputState.brightnessDown;

    float dt = deltaTime;
    std::thread solverThread([this, dt]() 
    { 
        while (true)
        {
            solver->Solve(dt, GetUniverse());
            simulationTime += deltaTime;
        }
    });

    glutMainLoop();

    return 0;
}

static void DrawBarnesHutTree(const BarnesHutTree& node)
{
	lpVec3 p = node.point;
	float l = node.length;

	glBegin(GL_LINE_STRIP);
		glColor3f(0.0f, 1.0f, 0.0f);

		glVertex3f(p.m_x, p.m_y, 0.0f);
		glVertex3f(p.m_x + l, p.m_y, 0.0f);
		glVertex3f(p.m_x + l, p.m_y + l, 0.0f);
		glVertex3f(p.m_x, p.m_y + l, 0.0f);
		glVertex3f(p.m_x, p.m_y, 0.0f);
	glEnd();

	if (!node.isLeaf)
	{
		for (int i = 0; i < 4; i++)
		{
            DrawBarnesHutTree(*node.children[i]);
		}
	}
}

void Application::OnDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    orbit.Transform();

    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    int mode = GLX_RENDER_DISK | (renderParams.renderTree ? GLX_RENDER_TREE : 0);

    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    lpVec3 v1 = lpVec3(modelview[0], modelview[4], modelview[8]);
    lpVec3 v2 = lpVec3(modelview[1], modelview[5], modelview[9]);

    
    //glDepthMask(false);

    if (renderParams.particleMode == RenderParameters::ParticleMode::Point)
    {
        glBegin(GL_POINTS);
        glColor3f(1.0f, 1.0f, 1.0f);
        for (auto& galaxy : universe->GetGalaxies())
        {
            for (auto& particle : galaxy.GetParticles())
            {
                if (!particle.active)
                {
                    continue;
                }

                lpVec3 pos = particle.position;

 
                //glColor3f(particle.color.m_x, particle.color.m_y, particle.color.m_z);
                glVertex3f(pos.m_x, pos.m_y, pos.m_z);
            }
        }
        glEnd();
    }
    else
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        for (auto& galaxy : universe->GetGalaxies())
        {
            for (auto& particlesByImage : galaxy.GetParticlesByImage())
            {
                assert(particlesByImage.first);
                glBindTexture(GL_TEXTURE_2D, particlesByImage.first->GetTextureId());

                glBegin(GL_QUADS);

                for (const auto* p : particlesByImage.second)
                {
                    assert(p);
                    const Particle& particle = *p;
                    if (!particle.active)
                    {
                        continue;
                    }

                    lpVec3 pos = particle.position;

                    float s = 0.5f * particle.size;

                    lpVec3 p1 = pos - v1 * s - v2 * s;
                    lpVec3 p2 = pos - v1 * s + v2 * s;
                    lpVec3 p3 = pos + v1 * s + v2 * s;
                    lpVec3 p4 = pos + v1 * s - v2 * s;

                    float mag = particle.magnitude * renderParams.brightness;
                    // Квадрат расстояние до частицы от наблюдателя
                    //float dist = (cameraX - pos.m_x) * (cameraX - pos.m_x) + (cameraY - pos.m_y) * (cameraY - pos.m_y) + (cameraZ - pos.m_z) * (cameraZ - pos.m_z);
                    ////dist = sqrt(dist);
                    //if (dist > 5.0f) dist = 5.0f;
                    //mag /= (dist / 2);

                    glColor3f(particle.color.m_x * mag, particle.color.m_y * mag, particle.color.m_z * mag);

                    glTexCoord2f(0.0f, 1.0f); glVertex3f(p1.m_x, p1.m_y, p1.m_z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(p2.m_x, p2.m_y, p2.m_z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(p3.m_x, p3.m_y, p3.m_z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(p4.m_x, p4.m_y, p4.m_z);

                    if (particle.doubleDrawing)
                    {
                        glTexCoord2f(0.0f, 1.0f); glVertex3fv(&p1.m_x);
                        glTexCoord2f(0.0f, 0.0f); glVertex3fv(&p2.m_x);
                        glTexCoord2f(1.0f, 0.0f); glVertex3fv(&p3.m_x);
                        glTexCoord2f(1.0f, 1.0f); glVertex3fv(&p4.m_x);
                    }


                    //glPopMatrix();
                }

                glEnd();
            }
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }

    //glDepthMask(true);

    if (renderParams.renderTree)
    {   
        DrawBarnesHutTree(universe->GetBarnesHutTree());
    }

    glutSwapBuffers();

    static uint64_t lastUpdate = 0;
    static int frames = 0;
    char buf[256];
    uint64_t currentTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    frames++;

    if (currentTime - lastUpdate >= 1000000000)
    {
        std::sprintf(buf, "%s [FPS: %d] [Simulation time: %.1f mln yrs]", cWindowCaption, frames, simulationTime * cMillionYearsInTimeUnit);
        glutSetWindowTitle(buf);
        frames = 0;
        lastUpdate = currentTime;
    }

    glutPostRedisplay();

    static int imageId = 0;
    /*if (started && saveToFiles)
        makeScreenShot(imageId++);*/
}

void Application::OnResize(int width, int height)
{
    this->width = width;
    this->height = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)width / (float)height, 0.001f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void Application::OnIdle()
{
    if (started)
    {
        //solver->Solve(delta_time, *universe);
    }

    if (inputState.brightnessUp)
    {
        renderParams.brightness = std::min(1.0f, renderParams.brightness * 1.1f);
    }
    if (inputState.brightnessDown)
    {
        renderParams.brightness = std::max(0.01f, renderParams.brightness / 1.1f);
    }
}

void Application::OnKeyboard(unsigned char key, int x, int y)
{
    if (key == 't')
    {
        renderParams.renderTree = !renderParams.renderTree;

        //ScreenShoter::GetScreenshot({ 0, 0, width, height }).SaveTga("Screenshot.tga");
    }
    if (key == ']') deltaTime *= 1.2f;
    if (key == '[') deltaTime *= 0.8f;
    if (key == ' ') started = false;
    if (key == 13)
    {
        if (!started) started = true;
    }

    if (key == 'u') curLayer++;

    for (auto& mapping : inputMappings)
    {
        if (mapping.first == key)
        {
            *mapping.second = true;
        }
    }
}

void Application::OnKeyboardUp(unsigned char key, int x, int y)
{
    if (key == 'm')
    {
        if (renderParams.particleMode == RenderParameters::ParticleMode::Billboard)
        {
            renderParams.particleMode = RenderParameters::ParticleMode::Point;
        } 
        else if (renderParams.particleMode == RenderParameters::ParticleMode::Point)
        {
            renderParams.particleMode = RenderParameters::ParticleMode::Billboard;
        }
    }

    for (auto& mapping : inputMappings)
    {
        if (mapping.first == key)
        {
            *mapping.second = false;
        }
    }
}

void Application::OnMousePressed(int button, int state, int x, int y)
{
    auto flag = 1u << button;
    if (state == GLUT_DOWN)
    {
        inputState.buttons |= flag;
        inputState.prevPos = {float(x), float(y)};
    }
    else
    {
        inputState.buttons &= ~flag;
    }
}

void Application::OnMouseMove(int x, int y)
{
    float2 delta = {float(x) - inputState.prevPos.x, float(y) - inputState.prevPos.y};
    inputState.prevPos = {float(x), float(y)};

    if (!inputState.buttons)
    {
        return;
    }

    if (inputState.buttons & (1u << GLUT_LEFT_BUTTON))
    {
        orbit.Rotate(delta.x * 0.1f, delta.y * 0.1f);
    }
    if (inputState.buttons & (1u << GLUT_MIDDLE_BUTTON))
    {
        orbit.Pan(-delta.x * orbit.GetDistance() * 0.001f, delta.y * orbit.GetDistance() * 0.001f);
    }
    if (inputState.buttons & (1u << GLUT_RIGHT_BUTTON))
    {
        orbit.MoveForward(delta.y * orbit.GetDistance() * 0.005f);
    }
}

void Application::OnMouseWheel(int button, int dir, int x, int y)
{
    orbit.MoveForward(-dir * orbit.GetDistance() * 0.1f);
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
    //FILE *f = fopen(fname, "r");
    //if (f == NULL)
    //    perror("Error opening file");

    //char line[256];
    //char lexem[64];
    //Galaxy *galaxy;
    //int i, j;
    //char c;
    //int linecount = 0;
    //float value;

    //while (feof(f) == 0)
    //{
    //    fgets(line, 255, f);
    //    linecount++;

    //    // Пропускаем коммент и пустые строки
    //    if (line[0] == '#' || line[0] == 10 || line[0] == 13)
    //        continue;

    //    if (strncmp(line, "[GALAXY]", 8) == 0)
    //    {
    //        if (!universe)
    //        {
    //            universe = new Universe(universeSize);
    //        }

    //        galaxy = new Galaxy();
    //        universe->CreateGalaxy(galaxy);
    //    }
    //    else
    //    {
    //        c = 1;
    //        i = 0;
    //        while (true)
    //        {
    //            // Обработка лексем

    //            char varname[64];
    //            getLexem(line, varname, i);
    //            if (varname[0] == 0)
    //                break;

    //            getLexem(line, lexem, i);
    //            if (lexem[0] != '=')
    //            {
    //                printf("Строка %d: ожидалось '='.", linecount);
    //                return false;
    //            }

    //            getLexem(line, lexem, i);

    //            value = atof(lexem);


    //            if (strcmp(varname, "DT") == 0)
    //            {
    //                DT = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "SAVE_FRAMES") == 0)
    //            {
    //                saveToFiles = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "UNIVERSE_SIZE") == 0)
    //            {
    //                universeSize = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_BULGE_NUM") == 0)
    //            {
    //                galaxy->numBulgeStars = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_DISK_NUM") == 0)
    //            {
    //                galaxy->numDiskStars = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_DISK_RADIUS") == 0)
    //            {
    //                galaxy->diskRadius = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_BULGE_RADIUS") == 0)
    //            {
    //                galaxy->bulgeRadius = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_HALO_RADIUS") == 0)
    //            {
    //                galaxy->haloRadius = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_DISK_THICKNESS") == 0)
    //            {
    //                galaxy->diskThickness = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_STAR_MASS") == 0)
    //            {
    //                galaxy->starMass = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_BULGE_MASS") == 0)
    //            {
    //                galaxy->bulgeMass = value;
    //                break;
    //            }
    //            else if (strcmp(varname, "GLX_HALO_MASS") == 0)
    //            {
    //                galaxy->haloMass = value;
    //                break;
    //            }
    //            else break;



    //        }





    //    }


    //}



    //fclose(f);

    return true;
}