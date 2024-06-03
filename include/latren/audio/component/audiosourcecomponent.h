#pragma once

#include <latren/ec/component.h>
#include <latren/systems.h>
#include "../audiosource.h"

class AudioSourceComponent : public Component<AudioSourceComponent> {
private:
    AudioSourceHandle source_;
public:
    AudioBufferHandle buffer; LE_RCDV(buffer)
    bool playAutomatically = false; LE_RCDV(playAutomatically)
    bool repeat = false; LE_RCDV(repeat)
    bool copyTransform = true; LE_RCDV(copyTransform)
    AudioSourceRelativeTo relativeTo = AudioSourceRelativeTo::WORLD_SPACE; LE_RCDV(relativeTo)

    void Start() override;
    void FixedUpdate() override;
    void Delete() override;
    AudioSourceHandle& GetAudioSource();
};