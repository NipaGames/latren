#pragma once

#include "audiosource.h"
#include "audiobuffer.h"

#include <latren/latren.h>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

class Camera;

class  AudioPlayer {
private:
    ALCdevice* alcDevice_ = nullptr;
    ALCcontext* alcContext_ = nullptr;
    std::vector<ALuint> buffers_;
    std::vector<ALuint> sources_;
public:
    virtual ~AudioPlayer();
    bool Init();
    AudioBufferHandle CreateAudioBuffer(const AudioBufferData&);
    AudioSourceHandle CreateAudioSource();
    void DeleteBuffer(AudioBufferHandle&);
    void DeleteSource(AudioSourceHandle&);
    void DeleteAllBuffers();
    void DeleteAllSources();
    void Destroy();
    void UseCameraTransform(const Camera&);
};