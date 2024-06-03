#include <latren/audio/audiosource.h>

void AudioSourceHandle::BindBuffer(const AudioBufferHandle& buffer) {
    alSourcei(handle_, AL_BUFFER, buffer.GetALHandle());
}

void AudioSourceHandle::Play() {
    alSourcePlay(handle_);
}

void AudioSourceHandle::Stop() {
    alSourceStop(handle_);
}

void AudioSourceHandle::Pause() {
    alSourcePause(handle_);
}

void AudioSourceHandle::ToggleRepeat(bool toggle) {
    alSourcei(handle_, AL_LOOPING, toggle ? AL_TRUE : AL_FALSE);
}

void AudioSourceHandle::SetRelativeTo(AudioSourceRelativeTo relativeTo) {
    switch(relativeTo) {
        case AudioSourceRelativeTo::LISTENER:
            alSourcei(handle_, AL_SOURCE_RELATIVE, AL_TRUE);
            break;
        case AudioSourceRelativeTo::WORLD_SPACE:
            alSourcei(handle_, AL_SOURCE_RELATIVE, AL_FALSE);
            break;
    }
}

void AudioSourceHandle::SetPosition(const glm::vec3& pos) {
    alSource3f(handle_, AL_POSITION, pos.x, pos.y, pos.z);
}