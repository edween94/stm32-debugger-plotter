#include "raylib.h"

#include "rlImGui.h"
#include "imgui.h"
#include "implot.h"

#include <vector>
#include <cmath>

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "STM32 Debugger + Plotter");
    SetTargetFPS(60);

    rlImGuiSetup(true);   // enable docking
    ImPlot::CreateContext();

    std::vector<float> t(1000), a(1000), b(1000);
    for (int i = 0; i < (int)t.size(); i++) {
        t[i] = i * 0.01f;
        a[i] = 1.0f + 0.25f * std::sinf(t[i] * 2.0f);
        b[i] = 0.8f + 0.20f * std::cosf(t[i] * 1.3f);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{20, 20, 20, 255});

        rlImGuiBegin();

        //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar)) {
            ImGui::TextUnformatted("Probe: ST-LINK   Interface: SWD   Target: STM32F4");
            ImGui::SameLine();
            if (ImGui::Button("Connect")) { /* TODO */ }

            ImGui::SameLine();
            if (ImGui::Button("Run")) { /* TODO */ }
            ImGui::SameLine();
            if (ImGui::Button("Halt")) { /* TODO */ }
            ImGui::SameLine();
            if (ImGui::Button("Step")) { /* TODO */ }

            ImGui::End();
        }

        if (ImGui::Begin("Live Signals")) {
            if (ImPlot::BeginPlot("##plot", ImVec2(-1, -1))) {
                ImPlot::SetupAxes("Time (s)", "Value");
                ImPlot::PlotLine("adc_filtered", t.data(), a.data(), (int)t.size());
                ImPlot::PlotLine("motor_rpm(norm)", t.data(), b.data(), (int)t.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
        }

        if (ImGui::Begin("Console")) {
            ImGui::TextWrapped("[App] UI scaffold running. Backend not wired yet.");
            ImGui::End();
        }

        rlImGuiEnd();
        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
