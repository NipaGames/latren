#pragma once

#include <latren/ec/component.h>
#include <latren/systems.h>
#include "../audiosource.h"

class AudioSourceComponent : public Component<AudioSourceComponent> {
private:
    AudioSourceHandle source_;
public:
    SERIALIZABLE(AudioBufferHandle, buffer);
    SERIALIZABLE(bool, playAutomatically) = false;
    SERIALIZABLE(bool, repeat) = false;
    SERIALIZABLE(bool, copyTransform) = true;
    SERIALIZABLE(AudioSourceRelativeTo, relativeTo) = AudioSourceRelativeTo::WORLD_SPACE;

    void Start() override;
    void FixedUpdate() override;
    void Delete() override;
    AudioSourceHandle& GetAudioSource();
};