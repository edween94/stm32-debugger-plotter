/* ======================== test_detector.cpp ========================= */
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "raylib.h"
#include "STM32Detector.h"

int main()
{
    ChangeDirectory(GetApplicationDirectory());
    printf("Testing STM32 Detector...\n");

    // Launch OpenOCD in background (exe is in build/bin, so ../../ to get to project root)
    #ifdef _WIN32
        const char* cmd = "start /B ../../tools/openocd/bin/openocd -s ../../tools/openocd/openocd/scripts -f interface/stlink.cfg -f target/stm32f4x.cfg >nul 2>&1";
    #else
        const char* cmd = "../../tools/openocd/bin/openocd -s ../../tools/openocd/openocd/scripts -f interface/stlink.cfg -f target/stm32f4x.cfg >/dev/null 2>&1 &";
    #endif

    printf("Starting OpenOCD...\n");
    system(cmd);

    // Give OpenOCD time to start
    std::this_thread::sleep_for(std::chrono::seconds(6));

    printf("Detecting STM32...\n");
    DetectionResult res = DetectedSTM32();

    if (res.success) {
        printf("SUCCESS!\n");
        printf("  Device ID: 0x%03X\n", res.devID);
        printf("  Config:    %s\n", res.configFileName.c_str());
    } else {
        printf("FAILED!\n");
        printf("  Error: %s\n", res.errMsg.c_str());
    }

    // Kill OpenOCD when done
    #ifdef _WIN32
        system("taskkill /F /IM openocd.exe >nul 2>&1");
    #else
        system("pkill openocd");
    #endif

    return res.success ? 0 : 1;
}