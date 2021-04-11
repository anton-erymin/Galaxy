#include "GalaxyEngine.h"

#include "Core/Solver.h"
#include "Core/Threading.h"
#include "Core/BarnesHutTree.h"

GalaxyEngine* g_instance = nullptr;

extern GL::ImagePtr g_shaded_image;
GL::PipelinePtr* g_particles_pipeline;

GalaxyEngine::GalaxyEngine(std::uint32_t width, std::uint32_t height, void* window_handle,
    const char* shaders_path)
    : Engine(width, height, window_handle, shaders_path)
{
    LOG("Galaxy Model 0.5\nCopyright (c) LAXE LLC 2012-2021");

    g_instance = this;

    ThreadPool::Create(std::thread::hardware_concurrency());

    //Config.draw_world_grid = true;
    //Config.draw_world_axes = true;
    //Config.draw_timing_plots = true;
    //Config.draw_debug_information = true;

    // Old initialization
    cSecondsPerTimeUnit = static_cast<float>(
        std::sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG)));
    cMillionYearsPerTimeUnit = cSecondsPerTimeUnit / 3600.0f / 24.0f / 365.0f / 1e+6f;

    deltaTime = 0.001f;
    deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;

    saveToFiles = false;

    Reset();

    particles_mesh_data_ = MeshData::Create();
    particles_vertex_buffer_ = GetNextEntity();
    GetRenderer().CreateDeviceBuffer("Particles mesh", particles_vertex_buffer_,
        20000000 * 4 * sizeof(float), GL::BufferType::kVertex, GL_DYNAMIC_DRAW);

    g_particles_pipeline = &particles_pipeline_;
}

GalaxyEngine& GalaxyEngine::GetInstance()
{
    assert(g_instance);
    return *g_instance;
}

void GalaxyEngine::Update(float time)
{
    ++frameCounter;

    static auto lastTime = std::chrono::high_resolution_clock::now();
    static float fpsTimer = 0.0f;
    static float frameTimer = 0.0f;

    auto now = std::chrono::high_resolution_clock::now();
    float time2 = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    frameTimer += time2;
    fpsTimer += time2;

    if (frameTimer > cFrameTime)
    {
        frameTimer -= cFrameTime;
    }

    if (fpsTimer >= 1.0f)
    {
        lastFps = frameCounter * (1.0f / fpsTimer);
        frameCounter = 0;
        fpsTimer = 0.0f;
    }

    simulationTimeMillionYears = simulationTime * cMillionYearsPerTimeUnit;
}

void GalaxyEngine::BuildUI()
{
}

static void DrawBarnesHutTree(const BarnesHutTree& node)
{
    float3 p = node.point;
    float l = node.length;

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINE_STRIP);

    glVertex3f(p.x, p.y, 0.0f);
    glVertex3f(p.x + l, p.y, 0.0f);
    glVertex3f(p.x + l, p.y + l, 0.0f);
    glVertex3f(p.x, p.y + l, 0.0f);
    glVertex3f(p.x, p.y, 0.0f);
    glEnd();

    if (!node.isLeaf)
    {
        for (int i = 0; i < 4; i++)
        {
            DrawBarnesHutTree(*node.children[i]);
        }
    }
}

void CreateParticlesPipelines(Renderer& r)
{
    GL::PipelineState state = {};
    state.SetColorAttachment(0, g_shaded_image);
    state.SetRootConstantsSize(sizeof(Device::ShadeSingleColorRootConstants));
    Device::ShadeSingleColorRootConstants root_constants = {};
    root_constants.color = float4(0.03f);
    *g_particles_pipeline = r.GetRenderDevice().CreatePipeline(
        "ShadeSingleColor", state, ShaderTools::Defines());
    (*g_particles_pipeline)->SetRootConstants(&root_constants);
}

void GalaxyEngine::PostRender()
{
    const auto& particles = universe->GetParticles();

    {
        if (renderParams.renderPoints)
        {
            const auto count = universe->position.size();

            if (!particles_pipeline_ && 
                g_shaded_image)
            {
                CreateParticlesPipelines(GetRenderer());
            }

            if (!write_flag_ && count > 0)
            {
                GetRenderer().GetDeviceBuffer(particles_vertex_buffer_)->
                    Write(0, count * sizeof(float4), universe->position.data());
                write_flag_ = true;
            }

            if (particles_pipeline_)
            {
                DEBUG_TIMING_BLOCK_GPU("Render particles");

                GL_CALL(glEnable(GL_BLEND));
                GL_CALL(glBlendFunc(GL_ONE, GL_ONE));

                particles_pipeline_->BeginGraphics(
                    GetRenderer().GetDeviceBuffer(particles_vertex_buffer_),
                    4 * sizeof(float));

                particles_pipeline_->Draw(0, count, GL_POINTS);

                particles_pipeline_->EndGraphics();

                GL_CALL(glDisable(GL_BLEND));
            }
        }
    }

#if 0
    

    if (renderParams.renderPoints)
    {
        glBegin(GL_POINTS);
        //glColor3f(1.0f, 1.0f, 1.0f);
        for (size_t i = 0; i < universe->GetParticlesCount(); i++)
        {
            const auto* particle = particles[i];
            if (!particle->active)
            {
                continue;
            }
            glColor3f(particle->color.x, particle->color.y, particle->color.z);
            float3 pos = universe->position[i];
            glVertex3f(pos.x, pos.y, pos.z);
        }
        glEnd();
    }
    else
#endif // 0

    {
#if 0
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
                // Squared distance to viewer
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
#endif // 0

}

    if (renderParams.renderTree)
    {
        std::lock_guard<std::mutex> lock(solverBarneshut->GetTreeMutex());
        DrawBarnesHutTree(solverBarneshut->GetBarnesHutTree());
    }

    if (renderParams.plotFunctions)
    {
        for (auto& galaxy : universe->GetGalaxies())
        {
            galaxy->GetHalo().PlotPotential();
        }
    }
}

void GalaxyEngine::Reset()
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
    //solverBarneshutGPU = std::make_unique<BarnesHutGPUSolver>(*universe);
    currentSolver = &*solverBarneshut;

    currentSolver->Inititalize(deltaTime);
    //currentSolver->SolveForces();
    //universe->SetRadialVelocitiesFromForce();
    currentSolver->Prepare();

    started = false;
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
