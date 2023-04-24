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
//#include <Private/RenderManager.h>
#include <UIOverlay.h>

GalaxySimulator::GalaxySimulator()
{
    NLOG("Galaxy Simulator 0.5\nCopyright (c) LAXE LLC 2012-2021");

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
    sim_context_.cMillionYearsPerTimeUnit = sim_context_.cSecondsPerTimeUnit / cSecondsPerHour / cHoursPerDay / cDaysPerYear / 1e+6f;

    // Setup context
    sim_context_.timestep = 0.000001f; //0.00001f
    sim_context_.algorithm = SimulationAlgorithm::BARNESHUT_CPU;
    sim_context_.gravity_softening_length = cSoftFactor;
    sim_context_.barnes_hut_opening_angle = cDefaultOpeningAngle;
    sim_context_.is_simulated = false;

    CreateUniverse();
    CreateRenderer();

    main_window_ = make_unique<MainWindow>(sim_context_, render_params_);
    BIND_EVENT_HANDLER(OnEvent);
    main_window_->AddEventHandler(*this);

    //engine->GetRenderer().GetRenderManager().SetVideoRecordState(true);

    CreateSolver(sim_context_.algorithm);
}

GalaxySimulator::~GalaxySimulator()
{
}

void GalaxySimulator::CreateUniverse()
{
    universe_ = make_unique<Universe>();

    //GalaxyParameters params = {};
    //params.disk_particles_count = 1;
    //universe_->CreateGalaxy(float3(), params);
    //universe_->CreateGalaxy(float3(0.2f, 0.0f, 0.0f), params);

    CreateGalaxy(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));

    //CreateGalaxy(float3(-1.5f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    //CreateGalaxy(float3(1.5f, 0.0f, -1.5f), float3(-1.0f, 0.0f, 0.0f));
}

void GalaxySimulator::CreateGalaxy(const float3& position, const float3& vel)
{
    // Save current count
    size_t cur_count = universe_->GetParticlesCount();

    GalaxyParameters params = {};
    params.disk_particles_count = 1;
    universe_->CreateGalaxy(position, params);

    universe_->masses_[cur_count] *= 100000.0f;
    universe_->velocities_[cur_count] = vel;

    auto AddSatellite = [&](int i)
    {
        float dist = RAND_RANGE(0.01f, 1.0f);
        float3 rand_dir(RAND_SNORM, 0.0f, RAND_SNORM);
        rand_dir.normalize();
        float3 ortho_dir = float3(rand_dir.z, 0.0f, -rand_dir.x);
        float3 pos = position + rand_dir * dist;
        pos.y = RAND_RANGE(-0.2f, 0.2f);
        GalaxyParameters params = {};
        params.disk_particles_count = 1;
        universe_->CreateGalaxy(pos, params);
        float vmag = RadialVelocity(universe_->masses_[cur_count], dist);
        universe_->velocities_[cur_count + i + 1] = vel;
        universe_->velocities_[cur_count + i + 1] += vmag * ortho_dir;
    };

    auto AddBody = [&](int i)
    {
        const float R = 10.0f;
        float3 rand_pos = float3(RAND_RANGE(-R, R), RAND_RANGE(-R, R), RAND_RANGE(-R, R));
        float3 pos = position + rand_pos;
        GalaxyParameters params = {};
        params.disk_particles_count = 1;
        universe_->CreateGalaxy(pos, params);
        universe_->velocities_[cur_count + i + 1] = vel;
    };

    for (size_t i = 0; i < 20000; i++)
    {
        AddSatellite(i);
        //AddBody(i);
    }
}

void GalaxySimulator::CreateSolver(SimulationAlgorithm algorithm)
{
    switch (algorithm)
    {
    case SimulationAlgorithm::BARNESHUT_CPU:
    {
        solver_.reset(new BarnesHutCPUSolver(*universe_, sim_context_, render_params_));
        solver_->Initialize();
        renderer_->SetUpdateHandler(nullptr);
        renderer_->SetPositionBuffer(nullptr);
        break;
    }
    case SimulationAlgorithm::BARNESHUT_GPU:
    {
        BarnesHutGPUSolver* solver = new BarnesHutGPUSolver(*universe_, sim_context_, render_params_);
        solver_.reset(solver);
        solver_->Initialize();
        renderer_->SetUpdateHandler(solver);
        renderer_->SetPositionBuffer(solver->GetPositionBuffer());
        break;
    }
    case SimulationAlgorithm::BRUTEFORCE_CPU:
    {
        solver_.reset(new BruteforceCPUSolver(*universe_, sim_context_, render_params_));
        solver_->Initialize();
        renderer_->SetUpdateHandler(nullptr);
        renderer_->SetPositionBuffer(nullptr);
        break;
    }
    case SimulationAlgorithm::BRUTEFORCE_GPU:
    {
        BruteforceGPUSolver* solver = new BruteforceGPUSolver(*universe_, sim_context_, render_params_);
        solver_.reset(solver);
        solver_->Initialize();
        renderer_->SetUpdateHandler(solver_.get());
        renderer_->SetPositionBuffer(solver->GetPositionBuffer());
        break;
    }
    default:
        break;
    }

    solver_->Start();
}

void GalaxySimulator::CreateRenderer()
{
    renderer_ = make_unique<GalaxyRenderer>(*universe_, sim_context_, render_params_);
    engine->GetRenderer().RegisterRendererPlugin(*renderer_);
}

void GalaxySimulator::OnEvent(Event& e)
{
    if (e.type == SID_DUP("AlgorithmChanged"))
    {
        CreateSolver(sim_context_.algorithm);
    }
}

#if 0
void GalaxySimulator::PostRender()
{

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


void GalaxySimulator::UpdateDeltaTime(float new_time)
{
    deltaTime = new_time;

    Device::ParticlesUpdateRootConstants consts = {};
    consts.time = deltaTime;
    particles_update_pipeline_->SetRootConstants(&consts);

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
