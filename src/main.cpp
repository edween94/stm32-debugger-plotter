#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "implot.h"

#include "SessionManager.h"

#include <vector>
#include <string>
#include <cmath>

//------------------------------------------------------------------------------
// Theme Setup
//------------------------------------------------------------------------------
static void SetupCyberpunkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]             = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.10f, 0.98f);

    colors[ImGuiCol_Border]               = ImVec4(0.00f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.00f, 0.60f, 0.60f, 0.30f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.00f, 0.80f, 0.80f, 0.40f);

    colors[ImGuiCol_TitleBg]              = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.00f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.02f, 0.02f, 0.03f, 0.80f);

    colors[ImGuiCol_MenuBarBg]            = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.05f, 0.05f, 0.06f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.00f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.70f, 0.70f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);

    colors[ImGuiCol_CheckMark]            = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.00f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);

    colors[ImGuiCol_Button]               = ImVec4(0.00f, 0.40f, 0.40f, 0.60f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.00f, 0.60f, 0.60f, 0.80f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.00f, 0.80f, 0.80f, 1.00f);

    colors[ImGuiCol_Header]               = ImVec4(0.00f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.00f, 0.60f, 0.60f, 0.70f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.00f, 0.80f, 0.80f, 0.90f);

    colors[ImGuiCol_Separator]            = ImVec4(0.00f, 0.50f, 0.50f, 0.30f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.00f, 0.70f, 0.70f, 0.60f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.00f, 1.00f, 1.00f, 0.90f);

    colors[ImGuiCol_Text]                 = ImVec4(0.85f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.40f, 0.45f, 0.45f, 1.00f);

    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 3.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 6.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;

    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;
}

static void SetupImPlotTheme() {
    ImPlotStyle& plotStyle = ImPlot::GetStyle();
    plotStyle.Colors[ImPlotCol_PlotBg]     = ImVec4(0.04f, 0.04f, 0.06f, 1.00f);
    plotStyle.Colors[ImPlotCol_PlotBorder] = ImVec4(0.00f, 0.50f, 0.50f, 0.30f);
    plotStyle.Colors[ImPlotCol_AxisGrid]   = ImVec4(0.00f, 0.40f, 0.40f, 0.15f);
    plotStyle.Colors[ImPlotCol_AxisTick]   = ImVec4(0.00f, 0.60f, 0.60f, 0.40f);
    plotStyle.Colors[ImPlotCol_AxisText]   = ImVec4(0.70f, 0.75f, 0.75f, 1.00f);

    plotStyle.LineWeight   = 2.0f;
    plotStyle.PlotPadding  = ImVec2(10, 10);
    plotStyle.Marker       = ImPlotMarker_None;
}

//------------------------------------------------------------------------------
// Status dot + text
//------------------------------------------------------------------------------
static void DrawConnectionStatus(ConnectionState conn, TargetState target)
{
    const char* text = "Disconnected";
    ImVec4 color = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);

    if (conn == ConnectionState::CONNECTING) {
        text = "Connecting...";
        color = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    } else if (conn == ConnectionState::ERROR) {
        text = "Error";
        color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    } else if (conn == ConnectionState::CONNECTED) {
        if (target == TargetState::RUNNING) {
            text = "Running";
            color = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
        } else if (target == TargetState::HALTED) {
            text = "Halted";
            color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        } else if (target == TargetState::STEPPING) {
            text = "Stepping";
            color = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        } else {
            text = "Connected";
            color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float r = 5.0f;

    dl->AddCircleFilled(
        ImVec2(pos.x + r, pos.y + ImGui::GetTextLineHeight() * 0.5f),
        r,
        ImGui::ColorConvertFloat4ToU32(color)
    );

    ImGui::Dummy(ImVec2(r * 2 + 6, 0));
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", text);
}

//------------------------------------------------------------------------------
// Toolbar row chooser
//------------------------------------------------------------------------------
static int GetToolbarRows(float winWidth)
{
    // These numbers are just "good enough" breakpoints.
    // You can tweak them later.
    if (winWidth >= 1500.0f) return 1;
    if (winWidth >= 1050.0f) return 2;
    return 3;
}

//------------------------------------------------------------------------------
// Toolbar parts
//------------------------------------------------------------------------------
static void DrawLeftControls(SessionManager& session)
{
    const bool connected  = (session.getConnectionState() == ConnectionState::CONNECTED);
    const bool connecting = (session.getConnectionState() == ConnectionState::CONNECTING);

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Probe");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110);

    static int probeIndex = 0;
    const char* probes[] = { "ST-LINK", "J-LINK", "CMSIS-DAP" };
    if (ImGui::Combo("##probe", &probeIndex, probes, IM_ARRAYSIZE(probes))) {
        session.SetProbeType(static_cast<ProbeType>(probeIndex));
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Interface");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);

    static int ifaceIndex = 0;
    const char* ifaces[] = { "SWD", "JTAG" };
    if (ImGui::Combo("##iface", &ifaceIndex, ifaces, IM_ARRAYSIZE(ifaces))) {
        session.SetDebugInterface(static_cast<DebugInterface>(ifaceIndex));
    }

    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Target");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110);

    static int targetIndex = 0;
    const char* targets[] = { "STM32F4", "STM32F1", "STM32H7", "STM32L4" };
    if (ImGui::Combo("##target", &targetIndex, targets, IM_ARRAYSIZE(targets))) {
        session.SetTargetDevice(targets[targetIndex]);
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(connecting);

    if (connected) {
        if (ImGui::Button("Disconnect")) session.disconnectFromTarget();
    } else {
        if (ImGui::Button("Connect")) session.connectToTarget();
    }

    ImGui::EndDisabled();

    ImGui::SameLine();
    DrawConnectionStatus(session.getConnectionState(), session.getTargetState());
}

static void DrawDebugControls(SessionManager& session)
{
    const bool connected = (session.getConnectionState() == ConnectionState::CONNECTED);
    ImGui::BeginDisabled(!connected);

    if (ImGui::SmallButton("Flash")) session.flashTarget();
    ImGui::SameLine();
    if (ImGui::SmallButton("Reset")) session.resetTarget();
    ImGui::SameLine();
    if (ImGui::SmallButton("Halt"))  session.haltTarget();
    ImGui::SameLine();

    bool halted = (session.getTargetState() == TargetState::HALTED);
    if (halted) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.55f, 0.35f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.75f, 0.45f, 0.90f));
    }
    if (ImGui::SmallButton("Run")) session.runTarget();
    if (halted) {
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Step In"))   session.stepInto();
    ImGui::SameLine();
    if (ImGui::SmallButton("Step Over")) session.stepOver();
    ImGui::SameLine();
    if (ImGui::SmallButton("Step Out"))  session.stepOut();

    ImGui::EndDisabled();
}

static void DrawElfControls(SessionManager& session, float minInputWidth)
{
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("ELF");
    ImGui::SameLine();

    static char elfPathBuf[256] = "firmware.elf";

    // Keep the input from collapsing too small.
    float w = ImGui::GetContentRegionAvail().x - 55.0f;
    if (w < minInputWidth) w = minInputWidth;

    ImGui::SetNextItemWidth(w);
    ImGui::InputText("##elf", elfPathBuf, sizeof(elfPathBuf));

    ImGui::SameLine();
    if (ImGui::SmallButton("Load")) {
        session.loadSymbolsFromElf(std::string(elfPathBuf));
        session.loadSymbols();
    }
}

//------------------------------------------------------------------------------
// Toolbar (responsive rows)
//------------------------------------------------------------------------------
static void DrawToolbar(SessionManager& session, int rows)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));

    if (rows == 1) {
        // 1 row: left | debug | elf
        if (ImGui::BeginTable("##ToolbarTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Left",   ImGuiTableColumnFlags_WidthStretch, 0.44f);
            ImGui::TableSetupColumn("Mid",    ImGuiTableColumnFlags_WidthStretch, 0.36f);
            ImGui::TableSetupColumn("Right",  ImGuiTableColumnFlags_WidthStretch, 0.20f);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            DrawLeftControls(session);

            ImGui::TableSetColumnIndex(1);
            DrawDebugControls(session);

            ImGui::TableSetColumnIndex(2);
            DrawElfControls(session, 180.0f);

            ImGui::EndTable();
        }
    }
    else if (rows == 2) {
        // 2 rows:
        // Row 1: left controls
        // Row 2: debug | elf
        DrawLeftControls(session);

        ImGui::Separator();

        if (ImGui::BeginTable("##ToolbarRow2", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Debug", ImGuiTableColumnFlags_WidthStretch, 0.65f);
            ImGui::TableSetupColumn("Elf",   ImGuiTableColumnFlags_WidthStretch, 0.35f);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            DrawDebugControls(session);

            ImGui::TableSetColumnIndex(1);
            DrawElfControls(session, 200.0f);

            ImGui::EndTable();
        }
    }
    else {
        // 3 rows:
        // Row 1: left controls
        // Row 2: debug controls
        // Row 3: elf
        DrawLeftControls(session);

        ImGui::Separator();
        DrawDebugControls(session);

        ImGui::Separator();
        DrawElfControls(session, 260.0f);
    }

    ImGui::PopStyleVar(3);
}

//------------------------------------------------------------------------------
// Panels
//------------------------------------------------------------------------------
static void DrawSidebar(SessionManager& /*session*/)
{
    if (ImGui::BeginTabBar("SidebarTabs")) {
        if (ImGui::BeginTabItem("Breakpoints")) { ImGui::TextDisabled("Coming soon..."); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Watch"))       { ImGui::TextDisabled("Coming soon..."); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Registers"))   { ImGui::TextDisabled("Coming soon..."); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Memory"))      { ImGui::TextDisabled("Coming soon..."); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Peripherals")) { ImGui::TextDisabled("Coming soon..."); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
}

static void DrawPlotPanel(SessionManager& /*session*/)
{
    static std::vector<float> t, a, b, c;
    if (t.empty()) {
        t.resize(600); a.resize(600); b.resize(600); c.resize(600);
        for (int i = 0; i < 600; i++) {
            float x = i * 0.012f;
            t[i] = x;
            a[i] = 0.2f + 0.9f * (1.0f / (1.0f + expf(-(x - 2.5f) * 3.0f))) + 0.02f * sinf(x * 10.0f);
            b[i] = 0.15f + 1.0f * (1.0f / (1.0f + expf(-(x - 3.0f) * 2.5f))) + 0.03f * sinf(x * 8.0f + 0.5f);
            c[i] = 1.5f - 0.3f * (1.0f / (1.0f + expf(-(x - 2.0f) * 4.0f))) + 0.2f * (1.0f / (1.0f + expf(-(x - 5.0f) * 2.0f))) + 0.01f * sinf(x * 15.0f);
        }
    }

    if (ImPlot::BeginPlot("##LiveSignals", ImVec2(-1, -1), ImPlotFlags_Crosshairs)) {
        ImPlot::SetupAxes("Time (s)", "Value");
        ImPlot::PlotLine("adc_filtered", t.data(), a.data(), (int)t.size());
        ImPlot::PlotLine("motor_rpm",     t.data(), b.data(), (int)t.size());
        ImPlot::PlotLine("battery_v",     t.data(), c.data(), (int)t.size());
        ImPlot::EndPlot();
    }
}

static void DrawConsolePanel(SessionManager& session)
{
    ImGui::BeginChild("ConsoleLog", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

    const auto& logs = session.getLogMessages();
    if (logs.empty()) {
        ImGui::TextDisabled("No logs yet. Click Connect to start.");
    } else {
        for (const auto& line : logs) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    static char cmdInput[256] = "";
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60);
    ImGui::InputTextWithHint("##cmd", "Enter command later...", cmdInput, sizeof(cmdInput));
    ImGui::SameLine();
    ImGui::Button("Send");
}

static void DrawStatusPanel(SessionManager& session)
{
    const auto& info = session.getTargetInfo();

    ImGui::TextUnformatted("Registers");
    ImGui::Separator();

    ImGui::Text("PC:");   ImGui::SameLine(60); ImGui::Text("0x%08X", info.pc);
    ImGui::Text("SP:");   ImGui::SameLine(60); ImGui::Text("0x%08X", info.sp);
    ImGui::Text("xPSR:"); ImGui::SameLine(60); ImGui::Text("0x%08X", info.xpsr);

    ImGui::Spacing();
    ImGui::TextUnformatted("Target");
    ImGui::Separator();

    ImGui::Text("Device:"); ImGui::SameLine(); ImGui::Text("%s", info.deviceName.c_str());

    ImGui::Text("Thread:");
    ImGui::SameLine();
    ImGui::Text("%s", info.currentThread.empty() ? "-" : info.currentThread.c_str());

    ImGui::Spacing();
    ImGui::Text("Load: %.0f%%", info.cpuLoad * 100.0f);
    ImGui::ProgressBar(info.cpuLoad, ImVec2(-1, 0), "");
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1600, 900, "STM32 Debugger + Plotter");
    SetTargetFPS(60);

    rlImGuiSetup(true);
    ImPlot::CreateContext();

    SetupCyberpunkTheme();
    SetupImPlotTheme();

    SessionManager session;
    session.initialize();

    const float sidebarWidth = 350.0f;
    const float statusPanelWidth = 220.0f;
    const float consoleHeight = 200.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        session.update(dt);

        BeginDrawing();
        ClearBackground(Color{ 15, 15, 20, 255 });

        rlImGuiBegin();

        float winWidth  = (float)GetScreenWidth();
        float winHeight = (float)GetScreenHeight();

        // Decide toolbar rows based on width
        int toolbarRows = GetToolbarRows(winWidth);

        // Make the toolbar height match the number of rows
        float oneRow = ImGui::GetFrameHeightWithSpacing()
                     + ImGui::GetStyle().WindowPadding.y * 2.0f
                     + 10.0f;
        float toolbarHeight = oneRow * (float)toolbarRows;

        // Top Toolbar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(winWidth, toolbarHeight));
        ImGui::Begin("##Toolbar", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
        DrawToolbar(session, toolbarRows);
        ImGui::End();

        // Left Sidebar
        ImGui::SetNextWindowPos(ImVec2(0, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth, winHeight - toolbarHeight - consoleHeight));
        ImGui::Begin("##Sidebar", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);
        DrawSidebar(session);
        ImGui::End();

        // Main Plot Area
        ImGui::SetNextWindowPos(ImVec2(sidebarWidth, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(winWidth - sidebarWidth - statusPanelWidth,
                                        winHeight - toolbarHeight - consoleHeight));
        ImGui::Begin("Live Signals", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse);
        DrawPlotPanel(session);
        ImGui::End();

        // Right Status Panel
        ImGui::SetNextWindowPos(ImVec2(winWidth - statusPanelWidth, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(statusPanelWidth, winHeight - toolbarHeight - consoleHeight));
        ImGui::Begin("Target Status", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse);
        DrawStatusPanel(session);
        ImGui::End();

        // Bottom Console
        ImGui::SetNextWindowPos(ImVec2(0, winHeight - consoleHeight));
        ImGui::SetNextWindowSize(ImVec2(winWidth, consoleHeight));
        ImGui::Begin("##Console", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);
        DrawConsolePanel(session);
        ImGui::End();

        rlImGuiEnd();
        EndDrawing();
    }

    session.shutdown();
    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
