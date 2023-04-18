#include "GalaxySimulator.h"
#include "Constants.h"
#include "Galaxy.h"
#include "Universe.h"
#include "Solvers/BruteforceCPUSolver.h"
#include "Solvers/BruteforceGPUSolver.h"
#include "Solvers/BarnesHutCPUSolver.h"
#include "Solvers/BarnesHutGPUSolver.h"
#include "GalaxyRenderer.h"
#include "MainWindow.h"
#include "MathUtils.h"

#include <Engine.h>
#include <EngineCore.h>
#include <Renderer.h>
#include <UIOverlay.h>

GalaxySimulator::GalaxySimulator()
{
    NLOG("Galaxy Model 0.5\nCopyright (c) LAXE LLC 2012-2021");

    // Setup ImGui context for this module
    UI_ACTIVATE_CONTEXT();

    // Create scene
    engine->SetActiveScene(engine->CreateScene());
    engine->Play();

    // Setup top camera
    Entity top_camera;
    CameraComponent* camera_comp = top_camera.Create<CameraComponent>();
    camera_comp->type = CameraComponent::Type::kOrtho;
    camera_comp->eye = float3(0.0f, 1.0f, 0.0f);
    camera_comp->at = float3();
    camera_comp->up = -Math::Z;
    //engine->SetActiveCamera(top_camera);

    Entity camera = engine->GetActiveCamera();
    camera.Get<CameraComponent>()->z_near = 0.000001f;
    camera.Get<CameraComponent>()->z_far = 100000000.0f;

    //srand(0);

    // Setup time measure units
    sim_context_.cSecondsPerTimeUnit = static_cast<float>(sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG)));
    sim_context_.cMillionYearsPerTimeUnit = sim_context_.cSecondsPerTimeUnit / 3600.0f / 24.0f / 365.0f / 1e+6f;

    // Setup context
    sim_context_.timestep = 0.00001f; //0.00001f
    sim_context_.algorithm = SimulationAlgorithm::BARNESHUT_CPU;
    sim_context_.gravity_softening_length = cSoftFactor;
    sim_context_.barnes_hut_opening_angle = cDefaultOpeningAngle;
    sim_context_.is_simulated = false;

    CreateUniverse();
    CreateSolver(sim_context_.algorithm);
    CreateRenderer();

    main_window_ = make_unique<MainWindow>(sim_context_, render_params_);

    solver_->Start();
}

GalaxySimulator::~GalaxySimulator()
{
}

void GalaxySimulator::CreateUniverse()
{
    universe_ = make_unique<Universe>();

    GalaxyParameters params = {};
    params.disk_particles_count = 1;
    universe_->CreateGalaxy(float3(), params);
    //universe_->CreateGalaxy(float3(0.2f, 0.0f, 0.0f), params);

    //universe_->SetRandomVelocities(0.2f, 0.3f);

    float r = 0.2f;

    universe_->masses_[0] *= 100000.0f;

    auto AddSatellite = [this](int i)
    {
        float dist = RAND_RANGE(0.01f, 1.0f);
        float3 rand_dir(RAND_SNORM, 0.0f, RAND_SNORM);
        rand_dir.normalize();
        float3 ortho_dir = float3(rand_dir.z, 0.0f, -rand_dir.x);
        float3 pos = rand_dir * dist;
        GalaxyParameters params = {};
        params.disk_particles_count = 1;
        universe_->CreateGalaxy(pos, params);
        float vmag = RadialVelocity(universe_->masses_[0], dist);
        universe_->velocities_[i] = vmag * ortho_dir;
    };

    float3 v0 = RadialVelocity(universe_->masses_[1], r) * float3(0.0f, 0.0f, 1.0f);
    float3 v1 = RadialVelocity(universe_->masses_[0], r) * float3(0.0f, 0.0f, -1.0f);

    //float f = universe_->masses_[0] * universe_->masses_[1] / (r * r);
    //float vmag = RadialVelocity(universe_->masses_[0], r);
    //vmag = RadialVelocity(f, universe_->masses_[1], r);

    //float3 v1 = RadialVelocity(f, universe_->masses_[1], r) * float3(0.0f, 0.0f, -1.0f);

    //universe_->velocities_[0] = v0 * 0.5f;
    //universe_->velocities_[1] = v1 * 0.5f;

    for (size_t i = 0; i < 10000; i++)
    {
        AddSatellite(i + 1);
    }
}

void GalaxySimulator::CreateSolver(SimulationAlgorithm algorithm)
{
    switch (algorithm)
    {
    case SimulationAlgorithm::BRUTEFORCE_CPU:
        solver_.reset(new BruteforceCPUSolver(*universe_, sim_context_, render_params_));
        break;
    case SimulationAlgorithm::BRUTEFORCE_GPU:
        //solver_.reset(new BruteforceGPUSolver(*universe_, sim_context_, render_params_));
        break;
    case SimulationAlgorithm::BARNESHUT_CPU:
        solver_.reset(new BarnesHutCPUSolver(*universe_, sim_context_, render_params_));
        break;
    case SimulationAlgorithm::BARNESHUT_GPU:
        //solver_.reset(new BarnesHutGPUSolver(*universe_, sim_context_, render_params_));
        break;
    default:
        break;
    }
}

void GalaxySimulator::CreateRenderer()
{
    renderer_ = make_unique<GalaxyRenderer>(*universe_, sim_context_, render_params_);
    engine->GetRenderer().RegisterRendererPlugin(*renderer_);
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

#if 0
void GalaxySimulator::Reset()
{
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
}
#endif // 0

#if 0
void GalaxySimulator::UpdateDeltaTime(float new_time)
{
    deltaTime = new_time;

    Device::ParticlesUpdateRootConstants consts = {};
    consts.time = deltaTime;
    particles_update_pipeline_->SetRootConstants(&consts);

}
#endif // 0

#if 0
void GalaxySimulator::Bind(GAL::GraphicsPipelinePtr& pipeline)
{

    pipeline->SetBuffer(GetRenderer().GetDeviceBuffer(nodes_buffer_), "NodesData");
    pipeline->SetBuffer(nodes_counter, "NodesCounter");
}
#endif // 0

#if 0
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
#endif // 0

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
engine->AddTimerAction(1.0f, false,
    [](float) -> bool
    {
        engine->RequestExit();
        return false;
    });
#endif // 0

#if 0

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
