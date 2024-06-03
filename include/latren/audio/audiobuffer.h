#pragma once

#include <cstdint>
#include "alhandle.h"

struct AudioBufferData {
    uint8_t* data = nullptr;
    int size = 0;
    int sampleRate = -1;
    int bitDepth = -1;
    int channels = -1;
    ALenum alFormat = -1;
};