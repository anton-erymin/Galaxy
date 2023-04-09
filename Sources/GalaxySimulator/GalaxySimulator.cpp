#include "GalaxySimulator.h"
#include "Universe.h"

#include <Engine.h>

//extern GAL::ImagePtr g_shaded_image;
//GAL::GraphicsPipelinePtr* g_particles_render_pipeline;
//GAL::GraphicsPipelinePtr* g_tree_draw_pipeline;

//ShaderTools::Defines g_defines;

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

#if 0
void GalaxySimulator::Update(float time)
{
    ++frameCounter;

    static auto lastTime = chrono::high_resolution_clock::now();
    static float fpsTimer = 0.0f;
    static float frameTimer = 0.0f;

    auto now = chrono::high_resolution_clock::now();
    float time2 = chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    frameTimer += time2;
    fpsTimer += time2;

    simulationTimeMillionYears = simulationTime * cMillionYearsPerTimeUnit;
}

#endif // 0

#if 0
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
#endif // 0

GalaxySimulator::GalaxySimulator()
{
    NLOG("Galaxy Model 0.5\nCopyright (c) LAXE LLC 2012-2021");

#if 1
    engine->SetActiveScene(engine->CreateScene());
    engine->AddBox();
    engine->AddPointLight(float3(1.0f, 2.0f, 0.0f), float3(50.0f));
#endif // 0

    engine->Play();

    // Old initialization
    cSecondsPerTimeUnit = static_cast<float>(
        sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG)));
    cMillionYearsPerTimeUnit = cSecondsPerTimeUnit / 3600.0f / 24.0f / 365.0f / 1e+6f;

    deltaTime = 0.0000001f;
    deltaTimeYears = deltaTime * cMillionYearsPerTimeUnit * 1e6f;

    CreateUniverse();

    saveToFiles = false;

    //Reset();

    //const auto count = universe->GetParticlesCount();

    //particles_buffer_ = g_engine_core->CreateEntity();

#if 0
    GetRenderer().CreateDeviceBuffer("Particles", particles_buffer_,
        count * sizeof(Device::Particle), GAL::BufferType::kStorage, GL_DYNAMIC_DRAW);

    vector<Device::Particle> device_particles(count);

    for (auto i = 0u; i < count; ++i)
    {
        device_particles[i].position_ = universe->position_[i];
        device_particles[i].position_.w = universe->particles_[i]->mass;
        device_particles[i].velocity.w = universe->inverseMass[i];
        device_particles[i].velocity = universe->velocity[i];
        device_particles[i].acceleration = universe->acceleration[i];
        device_particles[i].force = universe->force[i];
}

    GetRenderer().GetDeviceBuffer(particles_buffer_)->
        Write(0, count * sizeof(Device::Particle), device_particles.data());

    auto nodes_count = 5 * universe->GetParticlesCount();

    nodes_buffer_ = GetNextEntity();
    GetRenderer().CreateDeviceBuffer("Nodes", nodes_buffer_,
        nodes_count * sizeof(Device::Node), GAL::BufferType::kStorage, GL_DYNAMIC_DRAW);

    uint32_t counter = 0;
    nodes_counter = GetRenderer().GetRenderDevice().CreateBuffer("Nodes counter",
        GAL::BufferType::kStorage, sizeof(uint32_t), GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, &counter);

    g_particles_render_pipeline = &particles_render_pipeline_;
    g_tree_draw_pipeline = &tree_draw_pipeline_;

    camera_components[GetActiveCamera()]->z_far = 100000.0f;

    ui_ = make_unique<UI::GalaxyUI>(*this);

    {
        g_defines =
        {
            { "SOLVE_BRUTEFORCE", "" },
            { "PARTICLES_COUNT", to_string(count) },
            { "NODES_MAX_COUNT", to_string(nodes_count) },
            { "ROOT_RADIUS", to_string(GLX_UNIVERSE_SIZE * 0.5f) },
            { "SOFT_EPS", to_string(cSoftFactor) }
        };

        particles_solve_pipeline_ = GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesSolve.comp", {}, g_defines);

        particles_barnes_hut_pipeline_ = GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesBarnesHut.comp", {}, g_defines);

        GAL::PipelineState state = {};
        state.SetRootConstantsSize(sizeof(Device::ParticlesUpdateRootConstants));
        particles_update_pipeline_ = GetRenderer().GetRenderDevice().
            CreateComputePipeline("ParticlesUpdate.comp", state, g_defines);

        Bind(particles_barnes_hut_pipeline_.get());
        Bind(particles_solve_pipeline_.get());
        Bind(particles_update_pipeline_.get());
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
#endif // 0
}

GalaxySimulator::~GalaxySimulator()
{
}

void GalaxySimulator::CreateUniverse()
{
    universe = make_unique<Universe>(GLX_UNIVERSE_SIZE);
    universe->CreateGalaxy(float3(), GalaxyParameters());
}

void GalaxySimulator::CreateParticlesRenderPipelines()
{
#if 0
    GAL::PipelineState state = {};
    state.SetColorAttachment(0, g_shaded_image);
    state.SetRootConstantsSize(sizeof(Device::ShadeSingleColorRootConstants));
    Device::ShadeSingleColorRootConstants root_constants = {};

    *g_particles_render_pipeline = GetRenderer().GetRenderDevice().CreateGraphicsPipeline(
        "Particles.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag",
        state, ShaderTools::Defines());

    {
        *g_tree_draw_pipeline = GetRenderer().GetRenderDevice().CreateGraphicsPipeline(
            "DrawBarnesHut.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag",
            state, g_defines);
    }

    RenderUtils::BindSceneData(GetRenderer(), *g_particles_render_pipeline);
    RenderUtils::BindSceneData(GetRenderer(), *g_tree_draw_pipeline);
    Bind(particles_render_pipeline_.get());
    Bind(tree_draw_pipeline_.get());
#endif // 0

}

#if 0
void GalaxySimulator::PostRender()
{
    const auto& particles_ = universe->GetParticles();
    const auto count = universe->GetParticlesCount();
    auto nodes_count = 5 * universe->GetParticlesCount();

    if (is_simulated_)
    {
        auto group_count = CalcNumGroups(count, kGroupSize1D);

        {
            DEBUG_TIMING_BLOCK_GPU("Barnes-Hut");

            const auto kParticlesPerThread = 1u;
            particles_barnes_hut_pipeline_->Dispatch(CalcNumGroups(count / kParticlesPerThread, kGroupSize1D));
            GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

#if 0
            static vector<Device::Node> node_data(nodes_count);
            auto buf = renderer_->GetDeviceBuffer(nodes_buffer_);
            buf->Bind();
            void* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            memcpy(node_data.data(), p, buf->GetSize());
            GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
#endif // 0

#if 1
            {
                int result = -1;
                nodes_counter->Bind();
                void* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
                memcpy(&result, p, sizeof(result));
                GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
                int a = 1;
                //is_simulated_ = false;
            }
#endif // 0
        }

        {
            DEBUG_TIMING_BLOCK_GPU("Solve");

            //particles_solve_pipeline_->Dispatch(group_count);
            GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
        }

        {
            DEBUG_TIMING_BLOCK_GPU("Particles Update");

            particles_update_pipeline_->Dispatch(group_count);
            GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
        }
    }

    if (!particles_render_pipeline_ && g_shaded_image)
    {
        CreateParticlesRenderPipelines();
    }

    if (renderParams.renderPoints && particles_render_pipeline_)
    {
        DEBUG_TIMING_BLOCK_GPU("Render particles");

        //GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(GL_ONE, GL_ONE));
        GL_CALL(glPointSize(5.0f));

        Device::ShadeSingleColorRootConstants root_constants = {};
        //root_constants.color = float4(50000.0f / count);
        root_constants.color = float4(1.0f);
        root_constants.transform = matrix();
        particles_render_pipeline_->SetRootConstants(&root_constants);

        particles_render_pipeline_->BeginGraphics();
        particles_render_pipeline_->Draw(0, count, GL_POINTS);
        particles_render_pipeline_->EndGraphics();

        GL_CALL(glDisable(GL_BLEND));
    }

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
            const Particle& particle = *particles_[i];
            if (!particle.active)
            {
                continue;
            }

            float s = 0.5f * particle.size_ * renderParams.particlesSizeScale;

            float3 pos = universe->position_[i];

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

    if (renderParams.renderTree)
    {
        DEBUG_TIMING_BLOCK_GPU("Draw tree");

        Device::ShadeSingleColorRootConstants root_constants = {};
        root_constants.color = Utils::kGreenColor;
        root_constants.transform = matrix();
        tree_draw_pipeline_->SetRootConstants(&root_constants);

        tree_draw_pipeline_->BeginGraphics();
        tree_draw_pipeline_->Draw(0, nodes_count, GL_POINTS);
        tree_draw_pipeline_->EndGraphics();
    }

    if (renderParams.plotFunctions)
    {
        for (auto& galaxy : universe->GetGalaxies())
        {
            galaxy->GetHalo().PlotPotential();
        }
    }
}
#endif // 0

void GalaxySimulator::Reset()
{
#if 0
    if (started)
    {
        started = false;
        solverThread.join();
    }

    

    solverBruteforce = make_unique<BruteforceSolver>(*universe);
    solverBarneshut = make_unique<BarnesHutCPUSolver>(*universe);
    //solverBarneshutGPU = make_unique<BarnesHutGPUSolver>(*universe);
    currentSolver = &*solverBarneshut;

    currentSolver->Inititalize(deltaTime);
    currentSolver->SolveForces();
    universe->SetRadialVelocitiesFromForce();
    currentSolver->Prepare();
#endif // 0


    started = false;
#if 0
    solverThread = thread([this]()
        {
            while (started)
            {
                currentSolver->Solve(deltaTime);
                simulationTime += deltaTime;
                ++numSteps;
            }
        });
#endif // 0

}

void GalaxySimulator::UpdateDeltaTime(float new_time)
{
    deltaTime = new_time;

#if 0
    Device::ParticlesUpdateRootConstants consts = {};
    consts.time = deltaTime;
    particles_update_pipeline_->SetRootConstants(&consts);
#endif // 0

}

#if 0
void GalaxySimulator::Bind(GAL::GraphicsPipelinePtr& pipeline)
{
    pipeline->SetBuffer(GetRenderer().GetDeviceBuffer(particles_buffer_), "ParticlesData");
    pipeline->SetBuffer(GetRenderer().GetDeviceBuffer(nodes_buffer_), "NodesData");
    pipeline->SetBuffer(nodes_counter, "NodesCounter");
}
#endif // 0
