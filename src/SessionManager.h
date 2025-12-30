/* =============== SessionManager.h ==================
    Project: STM32 Debugger + Plotter
    Module: Session Manager

    Primary Author: Edwin Baiden
    Description:
        
*/


//Header guard
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

//Necessary libraries
#include <string>
#include <vector>
#include <cstdint> // For uint32_t
#include <memory> // For std::unique_ptr

/**
  * @brief Connection states for the debugging session
  * @author Edwin Baiden

  The connection states represent the various stages of connectivity between the debugger and the target device.
  - DISCONNECTED: No connection to the target device.
  - CONNECTING: Attempting to establish a connection to the target device.
  - CONNECTED: Successfully connected to the target device.
  - ERROR: An error occurred during the connection process.
*/
enum class ConnectionState {DISCONNECTED, CONNECTING, CONNECTED, ERROR};


/**
  * @brief Target states for the debugging session
  * @author Edwin Baiden

  The target states represent the current execution state of the target device being debugged. Its how 
  the code on the target device is currently behaving.
  - UNKNOWN: The state of the target device is unknown.
  - RUNNING: The target device is currently executing code.
  - HALTED: The target device execution is halted.
  - STEPPING: The target device is executing a single step.
  - ERROR: An error occurred in the target device.
*/
enum class TargetState {UNKNOWN, RUNNING, HALTED, STEPPING, ERROR};

/**
  * @brief Probe types for the debugging session
  * @author Edwin Baiden
  
  The probe types tell us what kind of hardware debugger we are using to connect to the target device.
  - STLINK: ST-Link debugger(what I will be using).
  - JLINK: J-Link debugger(The segger one (third party)).
  - CMSISDAP: CMSIS-DAP debugger(Another third party one, apparently open source??).
*/
enum class ProbeType {STLINK, J_LINK, CMSISDAP};

/**
  * @brief Debug interface types for the debugging session
  * @author Edwin Baiden

  The debug interface types specify the communication protocol used between the debugger and the target device.
  - SWD: Serial Wire Debug interface (SWDIO + SWCLK)
  - JTAG: Joint Test Action Group interface (standard 4/5-pin JTAG, i dont really know the specifics, but chatgpt said to include it??)
 */
enum class DebugInterface {SWD, JTAG};

// Data structure to hold application
/**
  
*/
struct AppConfig
{
    int windowWidth = 1920;
    int windowHeight = 1080;
    int targetFPS = 60;
    bool darkMode = true;

    std::string lastElfPath = "";
    std::string lastProbeType = "";

    float sampleRateHz =20.0f;
};

//Target device information
struct TargetDeviceInfo
{
    std::string deviceName = "STM32F4";

    uint32_t pc = 0; // Program Counter
    uint32_t sp = 0; // Stack Pointer
    uint32_t xpsr = 0; // Program Status Register

    std::string currentThread = "";
    TargetState state = TargetState::UNKNOWN;
    float cpuLoad = 0.0f;
};

// Data structure to hold plot signal data
struct PlotSignal 
{
    std::string name = "";
    std::vector<float> data;
    bool visible = true;
};

/**
    * @brief Manages the debugging session, including connection state, target state, and data plotting. This class 
    * handles the connection to the target device, manages the state of the debugging session, and stores data for 
    * live plotting. It provides methods to connect/disconnect, start/stop the target, and update plot data.

    * @author Edwin Baiden
    * @version 1.0
 */
class SessionManager
{
    private:
    
        AppConfig config; // Application configuration
        TargetDeviceInfo targetInfo; // Target device information


        // Session state variables
        ConnectionState connectionState = ConnectionState::DISCONNECTED;
        ProbeType probeType = ProbeType::STLINK;
        DebugInterface debugInterface = DebugInterface::SWD;


        

        // Plot data
        std::vector<float> timeData;
        std::vector<PlotSignal> plotSignals;

        std::string elfPath = "";
        bool symbolsLoaded = false;

        // Timers
        float connectionTimer = 0.0f;
        float simulationTime = 0.0f;

        // Log stuff
        std::vector<std::string> logMessages;
        void addLogMessage(const std::string& message);

        //std::unique_ptr<GDB_Client> gdbClient; // GDB Client for target communication
        //std::unique_ptr<SignalBuffer> signalBuffer; // Signal buffer for data plotting

    public:

        //Constructor and Destructor
        SessionManager();
        ~SessionManager();

        //Life cycle stuff
        bool initialize();
        void shutdown();

        //Connection management stuff
        bool connectToTarget();
        void disconnectFromTarget();
        ConnectionState getConnectionState() const {return this->connectionState;}

        //Target control stuff
        void flashTarget();
        void runTarget();
        void haltTarget();
        void resetTarget();
        void stepInto();
        void stepOver();
        void stepOut();

        bool loadSymbolsFromElf(const std::string& elfPath);
        bool loadSymbols();
        const std::string& getElfPath() const {return this->elfPath;}

        TargetState getTargetState() const {return this->targetInfo.state;}
        const TargetDeviceInfo& getTargetInfo() const {return this->targetInfo;}
        const AppConfig& getAppConfig() const {return this->config;}
        AppConfig& getAppConfigRef() {return this->config;}

        void SetProbeType(ProbeType type) {this->probeType = type;}
        void SetDebugInterface(DebugInterface inface) {this->debugInterface = inface;}
        void SetTargetDevice(const std::string& deviceName) {this->targetInfo.deviceName = deviceName;}

        ProbeType getProbeType() const {return this->probeType;}
        DebugInterface getDebugInterface() const {return this->debugInterface;}
        const std::string& getTargetDevice() const {return this->targetInfo.deviceName;}
        const std::vector<float>& getTimeData() const { return this->timeData; }
        const std::vector<PlotSignal>& getPlotSignals() const { return this->plotSignals; }

        void Log(const std::string& src, const std::string& level, const std::string& message);
        const std::vector<std::string>& getLogMessages() const {return this->logMessages;}

        void update(float delta);
};

#endif // SESSIONMANAGER_H