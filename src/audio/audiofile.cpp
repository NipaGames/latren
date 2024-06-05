#include <latren/io/resourcemanager.h>
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>
#include <latren/systems.h>
#include <latren/audio/audioplayer.h>

std::optional<AudioBufferHandle> Resources::AudioManager::LoadResource(const std::fs::path& path) {
    AudioBufferData bufferData;
    if (path.extension() == ".ogg") {
        int len = stb_vorbis_decode_filename(path.string().c_str(), &bufferData.channels, &bufferData.sampleRate, reinterpret_cast<short**>(&bufferData.data));
        if (len < 0)
            return std::nullopt;
        bufferData.size = len * 2 * bufferData.channels;
        bufferData.bitDepth = 16;
        bufferData.alFormat = bufferData.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    }
    else {
        return std::nullopt;
    }
    return Systems::GetAudioPlayer().CreateAudioBuffer(bufferData);
}