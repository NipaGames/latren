#include <latren/audio/audiosource.h>

void AudioSource::Play() {
    alSourcePlay(handle_);
}

void AudioSource::Pause() {
    alSourcePause(handle_);
}

void AudioSource::BindBuffer(const AudioBuffer& buffer) {
    alSourcei(handle_, AL_BUFFER, buffer.GetALHandle());
}