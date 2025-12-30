/* =============== SessionManager.cpp ==================
    Project: STM32 Debugger + Plotter
    Module: Session Manager

    Primary Author: Edwin Baiden
    Description:
        This is the session brain for the app.
        Right now it is mostly fake, but it is still useful.
        Later we will replace the fake parts with OpenOCD + GDB.
*/

#include "SessionManager.h"

#include <cmath>        // sinf, cosf
#include <sstream>      // stringstream
#include <iomanip>      // setw, setfill

// Helper for hex printing (just for nicer logs)
static std::string hex32(uint32_t v)
{
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << v;
    return ss.str();
}

// ------------------------------
// Constructor / Destructor
// ------------------------------
SessionManager::SessionManager()
{
    // Keep the constructor light, initialize() does the setup.
}

SessionManager::~SessionManager()
{
    // If you forget to call shutdown(), this is still safe.
    this->shutdown();
}

// ------------------------------
// Logging
// ------------------------------
void SessionManager::addLogMessage(const std::string& message)
{
    this->logMessages.push_back(message);

    // Keep it from growing forever
    const size_t maxLines = 400;
    if (this->logMessages.size() > maxLines)
    {
        this->logMessages.erase(this->logMessages.begin());
    }
}

void SessionManager::Log(const std::string& src, const std::string& level, const std::string& message)
{
    // Example: [GDB][INFO] Connected
    std::string line = "[" + src + "][" + level + "] " + message;
    this->addLogMessage(line);
}

// ------------------------------
// Lifecycle
// ------------------------------
bool SessionManager::initialize()
{
    // Reset session state
    this->connectionState = ConnectionState::DISCONNECTED;
    this->probeType = ProbeType::STLINK;
    this->debugInterface = DebugInterface::SWD;

    this->elfPath = "";
    this->symbolsLoaded = false;

    this->connectionTimer = 0.0f;
    this->simulationTime = 0.0f;

    // Reset target info
    this->targetInfo.deviceName = "STM32F4";
    this->targetInfo.pc = 0x08000000;
    this->targetInfo.sp = 0x20020000;
    this->targetInfo.xpsr = 0x01000000;
    this->targetInfo.currentThread = "main";
    this->targetInfo.state = TargetState::HALTED;
    this->targetInfo.cpuLoad = 0.0f;

    // Clear UI buffers
    this->logMessages.clear();
    this->timeData.clear();
    this->plotSignals.clear();

    // Add 2 default signals for the plot
    PlotSignal s1;
    s1.name = "adc_filtered";
    s1.visible = true;

    PlotSignal s2;
    s2.name = "motor_rpm(norm)";
    s2.visible = true;

    this->plotSignals.push_back(s1);
    this->plotSignals.push_back(s2);

    this->Log("App", "INFO", "SessionManager initialized.");
    this->Log("App", "INFO", "Not connected yet.");
    this->runTarget();

    return true;
}

void SessionManager::shutdown()
{
    if (this->connectionState == ConnectionState::CONNECTED ||
        this->connectionState == ConnectionState::CONNECTING)
    {
        this->disconnectFromTarget();
    }

    this->Log("App", "INFO", "SessionManager shutdown.");
}

// ------------------------------
// Connection management
// ------------------------------
bool SessionManager::connectToTarget()
{
    if (this->connectionState == ConnectionState::CONNECTED)
    {
        this->Log("App", "WARN", "Already connected.");
        return true;
    }

    if (this->connectionState == ConnectionState::CONNECTING)
    {
        this->Log("App", "INFO", "Already connecting...");
        return true;
    }

    // Fake connect
    this->connectionState = ConnectionState::CONNECTING;
    this->connectionTimer = 0.0f;

    this->Log("OpenOCD", "INFO", "Attempting connection (fake)...");
    return true;
}

void SessionManager::disconnectFromTarget()
{
    if (this->connectionState == ConnectionState::DISCONNECTED)
    {
        return;
    }

    // TODO later: close GDB, stop OpenOCD process
    this->connectionState = ConnectionState::DISCONNECTED;
    this->targetInfo.state = TargetState::UNKNOWN;

    this->Log("App", "INFO", "Disconnected from target.");
}

// ------------------------------
// Target control (still fake)
// ------------------------------
void SessionManager::flashTarget()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        this->Log("App", "ERROR", "Not connected to target.");
        return;
    }

    if (this->elfPath.empty())
    {
        this->Log("App", "ERROR", "No ELF file loaded.");
        return;
    }

    this->Log("GDB", "INFO", "Flashing target (not implemented yet)...");
    this->Log("GDB", "INFO", "Flash complete (fake).");
}

void SessionManager::resetTarget()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        this->Log("App", "ERROR", "Not connected to target.");
        return;
    }

    // Fake reset puts PC back to reset vector area
    this->targetInfo.pc = 0x08000000;
    this->targetInfo.state = TargetState::HALTED;

    this->Log("GDB", "INFO", "Reset target. PC=" + hex32(this->targetInfo.pc));
}

void SessionManager::haltTarget()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    this->targetInfo.state = TargetState::HALTED;
    this->Log("GDB", "INFO", "Halting target...");
}

void SessionManager::runTarget()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    this->targetInfo.state = TargetState::RUNNING;
    this->Log("GDB", "INFO", "Continuing execution...");
}

void SessionManager::stepInto()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    if (this->targetInfo.state != TargetState::HALTED)
    {
        this->Log("GDB", "WARN", "Cannot step because target is not halted.");
        return;
    }

    this->targetInfo.state = TargetState::STEPPING;

    // Fake step (thumb) moves PC by 2
    this->targetInfo.pc += 2;

    this->Log("GDB", "INFO", "Step into. PC=" + hex32(this->targetInfo.pc));

    this->targetInfo.state = TargetState::HALTED;
}

void SessionManager::stepOver()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    this->Log("GDB", "INFO", "Step over (same as stepInto for now).");
    this->stepInto();
}

void SessionManager::stepOut()
{
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    this->Log("GDB", "WARN", "Step out not implemented yet.");
}

// ------------------------------
// ELF / Symbols
// ------------------------------
bool SessionManager::loadSymbolsFromElf(const std::string& elfPath)
{
    this->elfPath = elfPath;
    this->symbolsLoaded = false;

    if (this->elfPath.empty())
    {
        this->Log("App", "ERROR", "ELF path is empty.");
        return false;
    }

    this->Log("App", "INFO", "ELF file set: " + this->elfPath);
    return true;
}

bool SessionManager::loadSymbols()
{
    if (this->elfPath.empty())
    {
        this->Log("App", "ERROR", "No ELF file specified.");
        return false;
    }

    this->Log("GDB", "INFO", "Loading symbols from " + this->elfPath + " (fake)");
    this->symbolsLoaded = true;
    this->Log("GDB", "INFO", "Symbols loaded successfully (fake).");
    return true;
}

// ------------------------------
// Update (call once per frame)
// ------------------------------
void SessionManager::update(float delta)
{
    if (delta < 0.0f) delta = 0.0f;

    // 1) Fake connection delay
    if (this->connectionState == ConnectionState::CONNECTING)
    {
        this->connectionTimer += delta;

        // After 1 second, say we are connected
        if (this->connectionTimer >= 1.0f)
        {
            this->connectionState = ConnectionState::CONNECTED;
            this->connectionTimer = 0.0f;

            this->targetInfo.state = TargetState::HALTED;

            this->Log("OpenOCD", "INFO", "Listening on port 3333 for gdb connections");
            this->Log("App", "INFO", "Connected to ST-LINK, target halted");
        }
        return;
    }

    // 2) If not connected, no updates
    if (this->connectionState != ConnectionState::CONNECTED)
    {
        return;
    }

    // 3) Simulate some registers even when halted (just to show something)
    // These values are fake but stable-looking
    if (this->targetInfo.state == TargetState::HALTED)
    {
        this->targetInfo.sp = 0x20020000;
        this->targetInfo.xpsr = 0x01000000;
        this->targetInfo.cpuLoad = 0.02f;
    }

    // 4) Only generate plot samples while RUNNING
    if (this->targetInfo.state != TargetState::RUNNING)
    {
        return;
    }

    this->simulationTime += delta;

    float rate = this->config.sampleRateHz;
    if (rate <= 0.0f) rate = 1.0f;

    // How many samples should exist at this time?
    int desiredCount = (int)std::floor(this->simulationTime * rate);

    // Add samples until we reach desiredCount
    while ((int)this->timeData.size() < desiredCount)
    {
        float t = (float)this->timeData.size() / rate;
        this->timeData.push_back(t);

        // Make sure every signal gets a value
        for (size_t i = 0; i < this->plotSignals.size(); i++)
        {
            float y = 0.0f;

            // Fake signal generation based on signal name
            if (this->plotSignals[i].name == "adc_filtered")
            {
                y = 1.0f + 0.25f * std::sinf(t * 2.0f);
            }
            else if (this->plotSignals[i].name == "motor_rpm(norm)")
            {
                y = 0.8f + 0.20f * std::cosf(t * 1.3f);
            }
            else
            {
                // Any extra signal still gets some data
                y = 0.5f + 0.1f * std::sinf(t * (1.0f + (float)i));
            }

            this->plotSignals[i].data.push_back(y);
        }

        // Fake PC moving while running
        this->targetInfo.pc += 4;
    }

    // Fake CPU load
    this->targetInfo.cpuLoad = 0.15f;

    // 5) Stop buffers from growing forever
    const int maxSamples = 3000;
    while ((int)this->timeData.size() > maxSamples)
    {
        this->timeData.erase(this->timeData.begin());

        for (size_t i = 0; i < this->plotSignals.size(); i++)
        {
            if (!this->plotSignals[i].data.empty())
            {
                this->plotSignals[i].data.erase(this->plotSignals[i].data.begin());
            }
        }
    }
}
