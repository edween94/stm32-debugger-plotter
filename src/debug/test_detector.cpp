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

    #ifdef _WIN32
        const char* cmd = "start /B ../../tools/openocd/bin/openocd -s ../../tools/openocd/openocd/scripts -f interface/stlink.cfg -f target/stm32wlx.cfg";
    #else
        const char* cmd = "../../tools/openocd/bin/openocd -s ../../tools/openocd/openocd/scripts -f interface/stlink.cfg -f target/stm32wlx.cfg &";
    #endif

    printf("Starting OpenOCD...\n");
    system(cmd);

    printf("Waiting for OpenOCD to initialize...\n");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    printf("Detecting STM32...\n");
    DetectionResult res = DetectedSTM32(4444);

    printf("--------------------------------------\n");
    if (res.success)
    {
        printf("SUCCESS!\n");
        printf("  Device ID: 0x%03X\n", res.devID);
        printf("  Config:    %s\n", res.configFileName.c_str());
    }
    else
    {
        printf("FAILED!\n");
        printf("  Error: %s\n", res.errMsg.c_str());
    }
    printf("--------------------------------------\n");

    #ifdef _WIN32
        system("taskkill /F /IM openocd.exe >nul 2>&1");
    #else
        system("pkill openocd");
    #endif

    return res.success ? 0 : 1;
}