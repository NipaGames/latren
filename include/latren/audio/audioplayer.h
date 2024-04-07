#pragma once

#include "audiosource.h"
#include "audiobuffer.h"

#include <latren/latren.h>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

class AudioPlayer {
private:
    ALCdevice* alcDevice_ = nullptr;
    ALCcontext* alcContext_ = nullptr;
    std::vector<ALuint> buffers_;
    std::vector<ALuint> sources_;
public:
    LATREN_API bool Init();
    LATREN_API AudioBuffer CreateAudioBuffer(const AudioBufferData&);
    LATREN_API AudioSource CreateAudioSource();
    LATREN_API void DeleteAllSources();
    LATREN_API void DeleteAllBuffers();
    LATREN_API void Destroy();
};