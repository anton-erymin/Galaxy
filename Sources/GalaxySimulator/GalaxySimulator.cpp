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
#include <ResourceManager.h>
#include <UIOverlay.h>
#include <Misc/Paths.h>

GalaxySimulator::GalaxySimulator()
{
    NLOG("Galaxy Simulator 0.5\nCopyright (c) LAXE LLC 2012-2021");

    // Setup ImGui context for this module
    UI_ACTIVATE_CONTEXT();

    // Manually assign name and load fonts
    ResourceManager::AssignNameToResource("Content/Assets/Fonts/exo2_medium_condensed.otf", SID("exo2_medium_condensed"));
    GET_UIOVERLAY().LoadFonts();

    ResourceManager::SetLocalDataSource(Paths::BaseDir() + "/../../../Data", ResourceDataSourceType::FILESYSTEM_DIRECTORY);

    // Create scene
    engine->SetActiveScene(engine->CreateScene());

    // Setup top camera
    Entity top_camera;
    CameraComponent* camera_comp = top_camera.Create<CameraComponent>();
    camera_comp->type = CameraType::Ortho;
    camera_comp->eye = Float3(0.0f, 1.0f, 0.0f);
    camera_comp->at = Float3();
    camera_comp->up = -Math::Z;
    //engine->SetActiveCamera(top_camera);

    Entity camera = engine->GetActiveCamera();
    camera.Get<CameraComponent>()->z_near = 0.000001f;
    camera.Get<CameraComponent>()->z_far = 100000000.0f;

    // Setup time measure units
    sim_context_.cSecondsPerTimeUnit = static_cast<float>(sqrt(cKiloParsec * cKiloParsec * cKiloParsec / (cMassUnit * cG)));
    sim_context_.cMillionYearsPerTimeUnit = sim_context_.cSecondsPerTimeUnit / cSecondsPerHour / cHoursPerDay / cDaysPerYear / 1e+6f;

    // Setup context
    sim_context_.timestep = 0.00001f;
    sim_context_.algorithm = SimulationAlgorithm::BARNESHUT_CPU;
    sim_context_.gravity_softening_length = cSoftFactor;
    sim_context_.barnes_hut_opening_angle = cDefaultOpeningAngle;
    sim_context_.is_simulated = false;

    CreateUniverse();
    CreateRenderer();

    main_window_ = MakeUnique<MainWindow>(sim_context_, render_params_);
    BIND_EVENT_HANDLER(OnEvent);
    main_window_->AddEventHandler(*this);

    //engine->GetRenderer().GetRenderManager().SetVideoRecordState(true);

    CreateSolver(sim_context_.algorithm);

    engine->Play();
}

GalaxySimulator::~GalaxySimulator()
{
}

void GalaxySimulator::CreateUniverse()
{
    universe_ = MakeUnique<Universe>();

    GalaxyParameters params = {};
    params.disk_particles_count = 30000;
    params.bulge_particles_count = 10000;
    params.total_mass = 1.0f;
    params.disk_radius = 1.0;
    params.disk_thickness = 0.1f;
    params.disk_mass_ratio = 1.0f;
    params.bulge_radius = 0.2f;
    params.halo_radius = 5.0f;
    params.black_hole_mass = 10000.0f;

    universe_->CreateGalaxy(Float3(), params);
    //universe_->CreateGalaxy(Float3(0.2f, 0.0f, 0.0f), params);

    //CreateGalaxy({}, {});
}

void GalaxySimulator::CreateGalaxy(const Float3& position, const Float3& vel)
{
    // Save current count
    size_t cur_count = universe_->GetParticlesCount();

    GalaxyParameters params = {};
    params.disk_particles_count = 1;
    universe_->CreateGalaxy(position, params);

    universe_->masses_[cur_count] *= 100000.0f;
    universe_->inverse_masses_[cur_count] /= 100000.0f;
    universe_->velocities_[cur_count] = vel;

    auto AddSatellite = [&](int i)
    {
        float dist = RandRange(0.001f, 1.0f);
        Float3 rand_dir(RandNormSigned(), 0.0f, RandNormSigned());
        rand_dir.Normalize();
        Float3 ortho_dir = Float3(rand_dir.z, 0.0f, -rand_dir.x);        
        Float3 pos = position + rand_dir * dist;
        pos.y = RandRange(-0.05f, 0.05f);
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
        Float3 rand_pos = Float3(RandRange(-R, R), RandRange(-R, R), RandRange(-R, R));
        Float3 pos = position + rand_pos;
        GalaxyParameters params = {};
        params.disk_particles_count = 1;
        universe_->CreateGalaxy(pos, params);
        universe_->velocities_[cur_count + i + 1] = vel;
    };

    for (size_t i = 0; i < 2000; i++)
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
        renderer_->SetUpdateHandler(solver);
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
    renderer_ = MakeUnique<GalaxyRenderer>(*universe_, sim_context_, render_params_);
    engine->GetRenderer().RegisterRendererPlugin(*renderer_);
}

void GalaxySimulator::OnEvent(Event& e)
{
    if (e.type == SID("AlgorithmChanged"))
    {
        CreateSolver(sim_context_.algorithm);
    }
}
