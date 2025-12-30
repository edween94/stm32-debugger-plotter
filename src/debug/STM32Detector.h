#ifndef STM32DETECTOR_H
#define STM32DETECTOR_H

#include <cstdint>
#include <string>

struct DetectionResult
{
    bool success = false;
    uint16_t devID = 0;
    std::string configFileName;
    std::string errMsg;
};

const char* getSTM32Config(uint16_t d_ID);
DetectionResult DetectedSTM32(int telnetPort = 4444);

#endif