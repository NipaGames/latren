#include <latren/audio/component/audiosourcecomponent.h>
#include <latren/systems.h>
#include <latren/ec/entity.h>
#include <latren/audio/audioplayer.h>
#include <latren/io/resourcemanager.h>

void AudioSourceComponent::Start() {
    source_ = Systems::GetAudioPlayer().CreateAudioSource();
    source_.BindBuffer(buffer);
    source_.ToggleRepeat(repeat);
    source_.SetRelativeTo(relativeTo);
    if (copyTransform)
        source_.SetPosition(parent.GetTransform().position);
    if (playAutomatically)
        source_.Play();
}

void AudioSourceComponent::FixedUpdate() {
    if (copyTransform && !parent.GetTransform().isStatic)
        source_.SetPosition(parent.GetTransform().position);
}

void AudioSourceComponent::Delete() {
    Systems::GetAudioPlayer().DeleteSource(source_);
}

AudioSourceHandle& AudioSourceComponent::GetAudioSource() {
    return source_;
}