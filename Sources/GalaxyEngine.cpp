#include "GalaxyEngine.h"

#include "Core/Solver.h"
#include "Core/Threading.h"
#include "Core/BarnesHutTree.h"
#include "RenderUtils.h"

GalaxyEngine* g_instance = nullptr;

extern GL::ImagePtr g_shaded_image;
GL::GraphicsPipelinePtr* g_particles_pipeline;

int3 CalcNumGroups(int size, uint group_size)
{
    return int3((size + group_size - 1) / group_size, 1, 1);
}

int3 CalcNumGroups(int2 size, uint group_size)
{
    return int3((size.x + group_size - 1) / group_size, (size.y + group_size - 1) / group_size, 1);
}

int3 CalcNumGroups(int3 size, uint group_size)
{
    return int3((size.x + group_size - 1) / group_size, (size.y + group_size - 1) / group_size,
        (size.z + group_size - 1) / group_size);
}

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

    deltaTime = 0.0000001f;
    deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;

    saveToFiles = false;

    Reset();

    particles_buffer_ = GetNextEntity();
    GetRenderer().CreateDeviceBuffer("Particles", particles_buffer_,
        20000000 * 4 * sizeof(float), GL::BufferType::kStorage, GL_DYNAMIC_DRAW);

    g_particles_pipeline = &particles_render_pipeline_;

    camera_components[GetActiveCamera()]->z_far = 100000.0f;
    
    ui_ = std::make_unique<UI::GalaxyUI>(*this);

    RebuildTimeDependentPipelines();

    {
        particles_clear_forces_pipeline_= GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesClearForce.comp");

        particles_clear_forces_pipeline_->SetBuffer(GetRenderer().GetDeviceBuffer(particles_buffer_),
            "ParticlesData");
    }

    {
        ShaderTools::Defines defines = { 
            { "SOLVE_BRUTEFORCE", "" }, { "N", std::to_string(universe->GetParticlesCount()) } };
        

        particles_solve_pipeline_ = GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesSolve.comp", {}, defines);

        particles_solve_pipeline_->SetBuffer(GetRenderer().GetDeviceBuffer(particles_buffer_),
            "ParticlesData");
    }

    controller_ = GetNextEntity();
    CreateInputComponent(controller_,
        [this](Engine&, const InputEvent& e, ObjectControllContext&) -> bool
        {
            if (e.type == InputEvent::Type::kKeyUp &&
                e.key == 'Q')
            {
                is_simulated_ = !is_simulated_;
                return true;
            }

            return false;

        })->is_active = true;
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
    ui_->Build();
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
    root_constants.color = float4(1.03f);
    root_constants.transform = matrix();
    *g_particles_pipeline = r.GetRenderDevice().CreateGraphicsPipeline(
        "Particles.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag",
        state, ShaderTools::Defines());
    (*g_particles_pipeline)->SetRootConstants(&root_constants);
   
    RenderUtils::BindSceneData(r, *g_particles_pipeline);
}

void GalaxyEngine::PostRender()
{
    const auto& particles = universe->GetParticles();

    {
        if (renderParams.renderPoints)
        {
            const auto count = universe->position.size();

            if (!particles_render_pipeline_ && 
                g_shaded_image)
            {
                CreateParticlesPipelines(GetRenderer());
            }

            if (!write_flag_ && count > 0)
            {
                std::vector<Device::Particle> device_particles(count);

                for (auto i = 0u; i < count; ++i)
                {
                    device_particles[i].position = universe->position[i];
                    device_particles[i].position.w = universe->particles[i]->mass;
                    device_particles[i].velocity.w = universe->inverseMass[i];
                    device_particles[i].velocity = universe->velocity[i];
                    device_particles[i].acceleration = universe->acceleration[i];
                    device_particles[i].force = universe->force[i];
                }

                GetRenderer().GetDeviceBuffer(particles_buffer_)->
                    Write(0, count * sizeof(Device::Particle), device_particles.data());
                write_flag_ = true;
            }

            if (is_simulated_)
            {
                {
                    particles_clear_forces_pipeline_->Dispatch(CalcNumGroups(count, kGroupSize1D));
                    GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                }

                {
                    //particles_barnes_hut_pipeline_->Dispatch(CalcNumGroups(count, Device::kGroupSize1D));
                    //GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                }

                {
                    particles_solve_pipeline_->Dispatch(CalcNumGroups(count, kGroupSize1D));
                    GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                }

                {
                    particles_update_pipeline_->Dispatch(CalcNumGroups(count, kGroupSize1D));
                    GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                }
            }

            if (particles_render_pipeline_)
            {
                DEBUG_TIMING_BLOCK_GPU("Render particles");

                GL_CALL(glEnable(GL_BLEND));
                GL_CALL(glBlendFunc(GL_ONE, GL_ONE));

                Device::ShadeSingleColorRootConstants root_constants = {};
                root_constants.color = float4(50000.0f / count);
                //root_constants.color = float4(1.0f);
                root_constants.transform = matrix();
                particles_render_pipeline_->SetRootConstants(&root_constants);

                particles_render_pipeline_->SetBuffer(GetRenderer().GetDeviceBuffer(particles_buffer_), 
                    "ParticlesData");

                particles_render_pipeline_->BeginGraphics();
                particles_render_pipeline_->Draw(0, count, GL_POINTS);
                particles_render_pipeline_->EndGraphics();

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
    currentSolver->SolveForces();
    universe->SetRadialVelocitiesFromForce();
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

void GalaxyEngine::UpdateDeltaTime(float new_time)
{
    deltaTime = new_time;

    Device::ParticlesUpdateRootConstants consts = {};
    consts.time = deltaTime;
    particles_update_pipeline_->SetRootConstants(&consts);

    RebuildTimeDependentPipelines();
}

void GalaxyEngine::RebuildTimeDependentPipelines()
{
    {
        GL::PipelineState state = {};
        ShaderTools::Defines defines = { { "TIME", std::to_string(deltaTime) } };
        state.SetRootConstantsSize(sizeof(Device::ParticlesUpdateRootConstants));
        particles_update_pipeline_ = GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesUpdate.comp", state, defines);

        particles_update_pipeline_->SetBuffer(GetRenderer().GetDeviceBuffer(particles_buffer_),
            "ParticlesData");

        //UpdateDeltaTime(deltaTime);
    }
}
