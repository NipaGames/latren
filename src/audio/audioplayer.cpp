#include <latren/audio/audioplayer.h>
#include <latren/graphics/camera.h>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>
#include <algorithm>

bool AudioPlayer::Init() {
    const char* alcDeviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
    alcDevice_ = alcOpenDevice(alcDeviceName);
    if (alcDevice_ == nullptr) {
        spdlog::error("OpenAL initialization failed!");
        return false;
    }
    alcContext_ = alcCreateContext(alcDevice_, nullptr);
    if (alcContext_ == nullptr) {
        spdlog::error("Couldn't create an OpenAL audio context!");
        return false;
    }
    alcMakeContextCurrent(alcContext_);
    return true;
}

AudioBufferHandle AudioPlayer::CreateAudioBuffer(const AudioBufferData& data) {
    ALuint bufferId;
    alGenBuffers(1, &bufferId);
    alBufferData(bufferId, data.alFormat, data.data, data.size, data.sampleRate);
    buffers_.push_back(bufferId);
    return AudioBufferHandle(bufferId);
}

AudioSourceHandle AudioPlayer::CreateAudioSource() {
    ALuint sourceId;
    alGenSources(1, &sourceId);
    sources_.push_back(sourceId);
    alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(sourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
    return AudioSourceHandle(sourceId);
}

void AudioPlayer::DeleteBuffer(AudioBufferHandle& buffer) {
    ALuint id = buffer.GetALHandle();
    alDeleteBuffers(1, &id);
    buffers_.erase(std::remove(buffers_.begin(), buffers_.end(), id), buffers_.end());
    buffer.Reset();
}

void AudioPlayer::DeleteSource(AudioSourceHandle& source) {
    ALuint id = source.GetALHandle();
    alDeleteSources(1, &id);
    sources_.erase(std::remove(sources_.begin(), sources_.end(), id), sources_.end());
    source.Reset();
}

void AudioPlayer::DeleteAllBuffers() {
    for (ALuint b : buffers_) {
        alDeleteBuffers(1, &b);
    }
    buffers_.clear();
}

void AudioPlayer::DeleteAllSources() {
    for (ALuint s : sources_) {
        alDeleteSources(1, &s);
    }
    sources_.clear();
}

void AudioPlayer::Destroy() {
    DeleteAllBuffers();
    DeleteAllSources();
    alcDevice_ = alcGetContextsDevice(alcContext_);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(alcContext_);
    alcCloseDevice(alcDevice_);
}

void AudioPlayer::UseCameraTransform(const Camera& cam) {
    alListener3f(AL_POSITION, cam.pos.x, cam.pos.y, cam.pos.z);
    glm::vec3 at = cam.front;
    glm::vec3 up = glm::normalize(glm::cross(cam.right, cam.front));
    ALfloat orientation[] = { at.x, at.y, at.z, up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, orientation);
}