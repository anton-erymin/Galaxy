#include "Application.h"
#include "Constants.h"
#include "Image.h"
#include "Solver.h"
#include "GPUSolver.h"
#include "Threading.h"
#include "BarnesHutTree.h"

#include <iostream>
#include <functional>
#include <chrono>
#include <fstream>

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

int Application::Run(int argc, char **argv)
{
    ThreadPool::Create(std::thread::hardware_concurrency());
    
    std::cout << "Galaxy Model 0.1\nCopyright (c) Laxe Studio 2012-2019" << std::endl << std::endl;

    imageLoader = std::make_unique<ImageLoader>();

    GetImageLoader().Load("Star", "Data/star.png");
    GetImageLoader().Load("Dust1", "Data/dust1.png");
    GetImageLoader().Load("Dust2", "Data/dust2.png");
    GetImageLoader().Load("Dust3", "Data/dust3.png");

    cSecondsPerTimeUnit = static_cast<float>(std::sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG)));
    cMillionYearsPerTimeUnit = cSecondsPerTimeUnit / 3600.0f / 24.0f / 365.0f / 1e+6f;

    deltaTime = 0.00000005f;
    deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;

    saveToFiles = false;

    srand(static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

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
    glutSpecialFunc([](int key, int x, int y) { Application::GetInstance().OnKeyboardSpecialFunc(key, x, y); });
    glutPassiveMotionFunc([](int x, int y) { Application::GetInstance().OnMousePassiveMove(x, y); });

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBlendFunc(GL_ONE, GL_ONE);

    GetImageLoader().GenTextureIds();

    inputMappings['a'] = &inputState.brightnessUp;
    inputMappings['z'] = &inputState.brightnessDown;

    ui.Init();
    ui.Text("GPU", (const char*)glGetString(GL_RENDERER));
    ui.ReadonlyFloat("FPS", &lastFps, 1);
    
    ui.Group("'Simulation parameters'");
    ui.ReadonlyInt("Number of particles", &totalParticlesCount);
    ui.ReadonlyFloat("Timestep", &deltaTime, 8);
    ui.ReadonlyFloat("Timestep, yrs", &deltaTimeYears);
    ui.ReadonlyFloat("Simulation time, mln yrs", &simulationTimeMillionYears);
    ui.ReadonlyInt("Number of time steps", &numSteps);
    ui.ReadonlyFloat("Build tree time, ms", &timings.buildTreeTimeMsecs, 1);
    ui.ReadonlyFloat("Solving time, ms", &timings.solvingTimeMsecs, 1);
    ui.Group("Rendering");
    ui.ReadonlyFloat("Camera distance, kpc", &orbit.GetDistance());
    ui.Checkbox("Render points", &renderParams.renderPoints, "m");
    ui.Checkbox("Render Barnes-Hut tree", &renderParams.renderTree, "t");
    ui.Checkbox("Plot potential", &renderParams.plotFunctions);
    ui.SliderFloat("Brightness", &renderParams.brightness, 0.05f, 10.0f, 0.01f);
    ui.SliderFloat("Particles size scale", &renderParams.particlesSizeScale, 0.01f, 10.0f, 0.01f);

    ui.Button("Fullscreen toggle", [](void*) 
    { 
        static bool fullscreen = false;
        if (!fullscreen)
        {
            glutFullScreen();
        }
        else
        {
            glutReshapeWindow(cWindowWidth, cWindowHeight);
        }
        fullscreen = !fullscreen;
    }, "ALT+RETURN");

    ui.Group("Model");
    ui.SliderFloat("Mass", &model.mass, 0.1f, 10000.0f, 1.0f);
    ui.SliderFloat("Disk mass ratio", &model.diskMassRatio, 0.0f, 1.0f, 0.01f);
    ui.SliderUint("Disk particles", &model.diskParticlesCount);
    ui.SliderUint("Bulge particles", &model.bulgeParticlesCount);
    ui.SliderFloat("Disk radius", &model.diskRadius, 0.01f, 10000.0f, 0.01f);
    ui.SliderFloat("Bulge radius", &model.bulgeRadius, 0.01f, 10000.0f, 0.01f);
    ui.SliderFloat("Halo radius", &model.haloRadius, 0.01f, 10000.0f, 0.01f);
    ui.SliderFloat("Disk thickness", &model.diskThickness, 0.0f, 100.0f, 0.01f);
    ui.SliderFloat("Black hole mass", &model.blackHoleMass, 1.0f, 10000.0f, 10.0f);
    ui.Checkbox("Dark matter", &simulationParams.darkMatter, "d");

    ui.Button("Apply", [](void*) 
    {
    });

    ui.Button("Reset", [](void*) 
    {
        Application::GetInstance().Reset();
    }, "F5");

    Reset();

    glutMainLoop();

    return 0;
}

void Application::Reset()
{
    if (started)
    {
        started = false;
        solverThread.join();
    }

    universe = std::make_unique<Universe>(GLX_UNIVERSE_SIZE);
    universe->CreateGalaxy({}, model);
    totalParticlesCount = static_cast<int32_t>(universe->GetParticlesCount());

    solverBruteforce = std::make_unique<BruteforceSolver>(*universe);
    solverBarneshut = std::make_unique<BarnesHutCPUSolver>(*universe);
    solverBarneshutGPU = std::make_unique<BarnesHutGPUSolver>(*universe);
    currentSolver = &*solverBarneshutGPU;

    currentSolver->Inititalize(deltaTime);
    currentSolver->SolveForces();
    universe->SetRadialVelocitiesFromForce();
    currentSolver->Prepare();

    started = true;
    solverThread = std::thread([this]() 
    {     
        while (started)
        {
            currentSolver->Solve(deltaTime);
            simulationTime += deltaTime;
            ++numSteps;
        }
    });
}

static void DrawBarnesHutTree(const BarnesHutTree& node)
{
	float3 p = node.point;
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
    ++frameCounter;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    orbit.Transform();

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    float3 v1 = float3(modelview[0], modelview[4], modelview[8]);
    float3 v2 = float3(modelview[1], modelview[5], modelview[9]);

    const auto particles = universe->GetParticles();

    if (renderParams.renderPoints)
    {
        glBegin(GL_POINTS);
        for (size_t i = 0; i < universe->GetParticlesCount(); i++)
        {
            const auto* particle = particles[i];
            if (!particle->active)
            {
                continue;
            }
            glColor3f(particle->color.m_x, particle->color.m_y, particle->color.m_z);
            float3 pos = universe->position[i];
            glVertex3f(pos.m_x, pos.m_y, pos.m_z);
        }
        glEnd();
    }
    else
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        //glDisable(GL_ALPHA_TEST);

        for (auto& particlesByImage : universe->GetParticlesByImage())
        {
            assert(particlesByImage.first);
            glBindTexture(GL_TEXTURE_2D, particlesByImage.first->GetTextureId());

            glBegin(GL_QUADS);

            for (const auto i : particlesByImage.second)
            {
                assert(i < universe->GetParticlesCount());
                const Particle& particle = *particles[i];
                if (!particle.active)
                {
                    continue;
                }

                float s = 0.5f * particle.size * renderParams.particlesSizeScale;

                float3 pos = universe->position[i];

                float3 p1 = pos - v1 * s - v2 * s;
                float3 p2 = pos - v1 * s + v2 * s;
                float3 p3 = pos + v1 * s + v2 * s;
                float3 p4 = pos + v1 * s - v2 * s;

                float magnitude = particle.magnitude * renderParams.brightness;
                // Квадрат расстояние до частицы от наблюдателя
                //float dist = (cameraX - pos.m_x) * (cameraX - pos.m_x) + (cameraY - pos.m_y) * (cameraY - pos.m_y) + (cameraZ - pos.m_z) * (cameraZ - pos.m_z);
                ////dist = sqrt(dist);
                //if (dist > 5.0f) dist = 5.0f;
                //mag /= (dist / 2);

                glColor3f(particle.color.m_x * magnitude, particle.color.m_y * magnitude, particle.color.m_z * magnitude);

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
            }

            glEnd();
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }

    if (renderParams.renderTree)
    {   
        std::lock_guard<std::mutex> lock(solverBarneshutGPU->GetTreeMutex());
        DrawBarnesHutTree(solverBarneshutGPU->GetBarnesHutTree());
    }

    glPopMatrix();

    if (renderParams.plotFunctions)
    {
        for (auto& galaxy : universe->GetGalaxies())
        {
            galaxy->GetHalo().PlotPotential();
        }
    }

    ui.Draw();

    glutSwapBuffers();
}

void Application::OnIdle()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    static float fpsTimer = 0.0f;
    static float frameTimer = 0.0f;

    auto now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    frameTimer += time;
    fpsTimer += time;

    if (frameTimer > cFrameTime)
    {
        glutPostRedisplay();
        frameTimer -= cFrameTime;
    }

    if (fpsTimer >= 1.0f)
    {
        lastFps = frameCounter * (1.0f / fpsTimer);
        frameCounter = 0;
        fpsTimer = 0.0f;
    }

    simulationTimeMillionYears = simulationTime * cMillionYearsPerTimeUnit;

    orbit.Update(time);

    if (inputState.brightnessUp)
    {
        renderParams.brightness = std::min(10.0f, renderParams.brightness + time);
    }
    if (inputState.brightnessDown)
    {
        renderParams.brightness = std::max(0.01f, renderParams.brightness - time);
    }
}

void Application::OnResize(int width, int height)
{
    this->width = width;
    this->height = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)width / (float)height, 0.001f, 1000000.0f);
    glMatrixMode(GL_MODELVIEW);

    ui.OnWindowSize(width, height);
}

void Application::OnKeyboard(unsigned char key, int x, int y)
{
    if (ui.OnKeyboard(key, x, y))
    {
        return;
    }

    if (key == ']')
    {
        deltaTime *= 1.2f;
        deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;
    }
    if (key == '[')
    {
        deltaTime *= 0.8f;
        deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;
    }

    if (inputMappings.count(key))
    {
        *inputMappings[key] = true;
    }
}

void Application::OnKeyboardUp(unsigned char key, int x, int y)
{
    if (inputMappings.count(key))
    {
        *inputMappings[key] = false;
    }
}

void Application::OnMousePressed(int button, int state, int x, int y)
{
    if (ui.OnMousePressed(button, state, x, y))
    {
        return;
    }

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
    if (ui.OnMousePassiveMove(x, y))
    {
        return;
    }

    float2 pos = {float(x), float(y)};
    float2 delta = pos - inputState.prevPos;
    inputState.prevPos = pos;

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
        orbit.MoveForward(delta.y * orbit.GetDistance() * 0.003f);
    }
}

void Application::OnMousePassiveMove(int x, int y)
{
    ui.OnMousePassiveMove(x, y);
}

void Application::OnMouseWheel(int button, int dir, int x, int y)
{
    if (ui.OnMouseWheel(button, dir, x, y))
    {
        return;
    }

    orbit.MoveForward(-dir * orbit.GetDistance() * 0.1f);
}

void Application::OnKeyboardSpecialFunc(unsigned char key, int x, int y)
{
    ui.OnSpecialFunc(key, x, y);
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