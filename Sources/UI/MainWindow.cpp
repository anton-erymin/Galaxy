#include "MainWindow.h"
#include <EngineMinimal.h>

MainWindow::MainWindow()
{
    window_ = engine->UISystem()->CreateUIWindow("Galaxy", true, int2(), int2(), [this](Entity e){ BuildUI(); })->entity;
    engine->UISystem()->ShowWindow(window_);
}

static void BuildTable(const char* name, function<void()>&& table_content)
{
    ImGui::PushID((string(name) + "TableHeader").c_str());

    if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTable((string(name) + "Table").c_str(), 2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);

            table_content();
        }

        ImGui::EndTable();
    }

    ImGui::PopID();
}

static void BuildRow(const char* name, function<void()>&& value_func)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text(name);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushID(name);
    value_func();
    ImGui::PopID();
}

template<typename T>
static void BuildRowValue(const char* name, const T& value)
{
    BuildRow(name, [&](){ ImGui::Text(to_string(value).c_str()); });
}

void MainWindow::BuildUI()
{
    bool render_points = true;
    bool render_tree = false;
    bool plot_potential = false;
    float brightness = 1.0f;
    float particle_size_scale = 1.0f;
    bool simulate_dark_matter = false;
    float gravity_softening_length = 1.0f;
    float mass = 1.0f;
    float disk_mass_ratio = 1.0f;
    int disk_particles_count = 1;
    int bulge_particles_count = 1;
    float disk_radius = 1.0f;
    float bulge_radius = 1.0f;
    float halo_radius = 1.0f;
    float disk_thickness = 1.0f;
    float black_hole_mass = 1.0f;

    float render_fps = 0.0f;

    float simulation_fps = 0.0f;
    float timestep = 0.0f;
    float timestep_yrs = 0.0f;
    float simulation_time = 0.0f;
    float simulation_time_million_yrs = 0.0f;
    size_t timesteps_count = 0;

    float build_tree_time_msecs = 0.0f;
    float solver_time_msecs = 0.0f;
    float total_step_time_msecs = 1.0f;

    struct SimulationParams
    {
        float simulation_fps = 0.0f;
        float timestep = 0.0f;
        float timestep_yrs = 0.0f;
        float simulation_time = 0.0f;
        float simulation_time_million_yrs = 0.0f;
        size_t timesteps_count = 0;

        float build_tree_time_msecs = 0.0f;
        float solver_time_msecs = 0.0f;
        float total_step_time_msecs = 1.0f;
    };

    BuildTable("Simulation",
        [&]()
        {
            BuildRowValue("Render FPS", render_fps);
            BuildRowValue("Simulation FPS", simulation_fps);
            BuildRowValue("Number of particles", 20);
            BuildRowValue("Timestep", timestep);
            BuildRowValue("Timestep, yrs", timestep_yrs);
            BuildRowValue("Simulation time", simulation_time);
            BuildRowValue("Simulation time, mln yrs", simulation_time_million_yrs);
            BuildRowValue("Number of time steps", timesteps_count);
            BuildRowValue("Build tree time, ms", build_tree_time_msecs);
            BuildRowValue("Solving time, ms", solver_time_msecs);
            BuildRowValue("Total step time, ms", total_step_time_msecs);

            BuildRow("Dark matter", [&](){ ImGui::Checkbox("", &simulate_dark_matter); });
            BuildRow("Gravity softness distance", [&](){ ImGui::SliderFloat("", &gravity_softening_length, 0.00001f, 0.1f, nullptr, 1.0f); });
        });

    BuildTable("Rendering",
        [&]()
        {
            BuildRowValue("Camera distance, kpc", 20);
            BuildRow("Render points", [&](){ ImGui::Checkbox("", &render_points); });
            BuildRow("Render Barnes-Hut tree", [&](){ ImGui::Checkbox("", &render_tree); });
            BuildRow("Plot potential", [&](){ ImGui::Checkbox("", &plot_potential); });
            BuildRow("Brightness", [&](){ ImGui::SliderFloat("", &brightness, 0.05f, 10.0f, nullptr, 1.0f); });
            BuildRow("Particles size scale", [&](){ ImGui::SliderFloat("", &particle_size_scale, 0.01f, 10.0f, nullptr, 1.0f); }); 
        });

    BuildTable("Model",
        [&]()
        {
            BuildRow("Mass", [&](){ ImGui::SliderFloat("", &mass, 0.1f, 10000.0f, nullptr, 1.0f); });
            BuildRow("Disk mass ratio", [&](){ ImGui::SliderFloat("", &disk_mass_ratio, 0.01f, 10.0f, nullptr, 1.0f); });
            BuildRow("Disk particles", [&](){ ImGui::SliderInt("", &disk_particles_count, 1, 10000, nullptr, 1.0f); });
            BuildRow("Bulge particles", [&](){ ImGui::SliderInt("", &bulge_particles_count, 1, 10000, nullptr, 1.0f); });
            BuildRow("Disk radius", [&](){ ImGui::SliderFloat("", &disk_radius, 0.01f, 10000.0f, nullptr, 1.0f); });
            BuildRow("Bulge radius", [&](){ ImGui::SliderFloat("", &bulge_radius, 0.01f, 10000.0f, nullptr, 1.0f); });
            BuildRow("Halo radius", [&](){ ImGui::SliderFloat("", &halo_radius, 0.01f, 10000.0f, nullptr, 1.0f); });
            BuildRow("Disk thickness", [&](){ ImGui::SliderFloat("", &disk_thickness, 0.0f, 100.0f, nullptr, 1.0f); });
            BuildRow("Black hole mass", [&](){ ImGui::SliderFloat("", &black_hole_mass, 1.0f, 10000.0f, nullptr, 1.0f); });
        });
}
