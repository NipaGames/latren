#pragma once

#include "audiobuffer.h"
#include <latren/latren.h>
#include <cstdint>

class AudioSource : public OpenAlHandle {
using OpenAlHandle::OpenAlHandle;
public:
    LATREN_API void BindBuffer(const AudioBuffer&);
    LATREN_API void Play();
    LATREN_API void Pause();
};