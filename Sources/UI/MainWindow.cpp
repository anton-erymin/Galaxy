#include "MainWindow.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <EngineMinimal.h>
#include <Renderer.h>
#include <UIOverlay.h>

MainWindow::MainWindow(SimulationContext& sim_context, RenderParameters& render_params)
    : sim_context_(sim_context)
    , render_params_(render_params)
{
    window_ = engine->UISystem()->CreateUIWindow("Galaxy Simulator", true, int2(), int2(), [this](Entity e){ BuildUI(); })->entity;
    engine->UISystem()->ShowWindow(window_);
    window_.Get<WindowUIComponent>()->color.w = 0.8f;
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
    ImGui::PushItemWidth(-FLT_MIN);
    value_func();
    ImGui::PopID();
}

template<typename T>
static void BuildRowValue(const char* name, const T& value)
{
    BuildRow(name, [&](){ ImGui::Text(to_string(value).c_str()); });
}

static void BuildRowStringValue(const char* name, const string& value)
{
    BuildRow(name, [&](){ ImGui::Text(value.c_str()); });
}

static string TimeInYearsToStr(float time_years)
{
    if (time_years < 1e+6f)
    {
        return to_string(time_years) + " yrs";
    }
    else if (time_years < 1e+9f)
    {
        return to_string(int(time_years * 1e-6f)) + " mln yrs";
    }
    else if (time_years < 1e+12f)
    {
        return to_string(int(time_years * 1e-9f)) + " mlrd yrs";
    }
    else
    {
        return to_string(int(time_years * 1e-12f)) + " trln yrs";
    }
}

void MainWindow::BuildUI()
{
    engine->GetRenderer().GetUIOverlay().PushFont(UI::UIOverlay::kRegularBold);

    float mass = 1.0f;
    float disk_mass_ratio = 1.0f;
    int disk_particles_count = 1;
    int bulge_particles_count = 1;
    float disk_radius = 1.0f;
    float bulge_radius = 1.0f;
    float halo_radius = 1.0f;
    float disk_thickness = 1.0f;
    float black_hole_mass = 1.0f;

    BuildTable("Simulation",
        [&]()
        {
            BuildRowValue("Render FPS", int(engine->GetFPSCounter().GetFPS()));
            BuildRowValue("Simulation FPS", int(sim_context_.simulation_fps));

            static const char* s_sim_types[] = { "Bruteforce CPU", "Bruteforce GPU", "Barnes-Hut CPU", "Barnes-Hut GPU" };
            BuildRow("Simulation type", [&](){ ImGui::Combo("", (int*)(&sim_context_.algorithm), s_sim_types, int(SimulationAlgorithm::MAX_COUNT)); });

            BuildRow("Simulation enabled", [&](){ ImGui::Checkbox("", &sim_context_.is_simulated); });
            //BuildRowValue("Number of particles", 20);
            BuildRow("Timestep", [&](){ ImGui::SliderFloat("", &sim_context_.timestep, 0.01f, 10.0f, nullptr, 1.0f); });
            BuildRowStringValue("Timestep, yrs", TimeInYearsToStr(sim_context_.timestep_yrs));
            BuildRowValue("Simulation time in units", sim_context_.simulation_time);
            BuildRowStringValue("Simulation time in yrs", TimeInYearsToStr(sim_context_.simulation_time_million_yrs * 1e+6f));
            BuildRowValue("Number of time steps", sim_context_.timesteps_count);
            BuildRowValue("Build tree time, ms", sim_context_.build_tree_time_msecs);
            BuildRowValue("Solving time, ms", sim_context_.solver_time_msecs);
            BuildRowValue("Total step time, ms", sim_context_.total_step_time_msecs);
            BuildRow("Dark matter", [&](){ ImGui::Checkbox("", &sim_context_.simulate_dark_matter); });
            BuildRow("Gravity softness distance", [&](){ ImGui::SliderFloat("", &sim_context_.gravity_softening_length, 0.0001f, 0.001f, nullptr, 1.0f); });
            BuildRow("Barnes-Hut Opening Angle", [&](){ ImGui::SliderFloat("", &sim_context_.barnes_hut_opening_angle, 0.1f, 1.0f, nullptr, 1.0f); });
            BuildRowValue("Tree nodes count", sim_context_.nodes_count);
        });

    BuildTable("Rendering",
        [&]()
        {
            BuildRowValue("Camera distance, kpc", 20);
            BuildRow("Render particles", [&](){ ImGui::Checkbox("", &render_params_.render_particles); });
            BuildRow("Render particles as points", [&](){ ImGui::Checkbox("", &render_params_.render_as_points); });
            BuildRow("Render Barnes-Hut tree", [&](){ ImGui::Checkbox("", &render_params_.render_tree); });
            BuildRow("Plot potential", [&](){ ImGui::Checkbox("", &render_params_.plot_potential); });
            BuildRow("Brightness", [&](){ ImGui::SliderFloat("", &render_params_.brightness, 0.05f, 10.0f, nullptr, 1.0f); });
            BuildRow("Particles size scale", [&](){ ImGui::SliderFloat("", &render_params_.particle_size_scale, 0.01f, 50.0f, nullptr, 1.0f); });
            BuildRow("Invert colors", [&](){ ImGui::Checkbox("", &render_params_.colors_inverted); });
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

    ImGui::PopFont();
}
