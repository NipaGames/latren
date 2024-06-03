#pragma once

#include "audiobuffer.h"
#include <latren/latren.h>
#include <latren/defines/opengl.h>
#include <cstdint>

enum class AudioSourceRelativeTo {
    LISTENER,
    WORLD_SPACE
};

class LATREN_API AudioSourceHandle : public OpenAlHandle {
using OpenAlHandle::OpenAlHandle;
public:
    void BindBuffer(const AudioBufferHandle&);
    void Play();
    void Stop();
    void Pause();
    void ToggleRepeat(bool);
    // by default relative to listener
    void SetRelativeTo(AudioSourceRelativeTo);
    void SetPosition(const glm::vec3&);
};