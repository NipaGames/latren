#include <latren/audio/audioplayer.h>

#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>

//check big vs little endian machine
static bool IsBigEndian(void) {
    int a = 1;
    return !((char*)&a)[0];
}

static int ConvertToInt(char* buffer, int len) {
    int a = 0;

    if(!IsBigEndian()) {
        for(int i = 0; i < len; ++i) {
            ((char*)&a)[i] = buffer[i];
        }
    }
    else {
        for(int i = 0; i < len; ++i) {
            ((char*)&a)[3-i] = buffer[i];
        }
    }

    return a;
}

//Location and size of data is found here: http://www.topherlee.com/software/pcm-tut-wavformat.html
AudioBufferData LoadWAV(const std::string& src) {
    AudioBufferData bufferData;
    char buffer[4];
    std::ifstream in(src, std::ios::binary);
    in.read(buffer, 4);
    if(strncmp(buffer, "RIFF", 4) != 0) {
        std::cout << "this is not a valid WAVE file" << std::endl;
        return bufferData;
    }
    in.read(buffer, 4);
    in.read(buffer, 4); //WAVE
    in.read(buffer, 4); //fmt 
    in.read(buffer, 4); //16
    in.read(buffer, 2); //1
    in.read(buffer, 2);
    bufferData.channels = ConvertToInt(buffer, 2);
    in.read(buffer, 4);
    bufferData.sampleRate = ConvertToInt(buffer, 4);
    in.read(buffer, 4);
    in.read(buffer, 2);
    in.read(buffer, 2);
    bufferData.bps = ConvertToInt(buffer, 2);
    in.read(buffer, 4); //data
    in.read(buffer, 4);
    bufferData.size = ConvertToInt(buffer, 4);
    bufferData.data = new uint8_t[bufferData.size];
    in.read((char*) bufferData.data, bufferData.size);

    bufferData.alFormat = ((bufferData.channels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8) | (bufferData.bps == 16);
    return bufferData; 
}

bool AudioPlayer::Init() {
    alcDevice_ = alcOpenDevice(NULL);
    if (alcDevice_ == nullptr) {
        spdlog::error("OpenAL initialization failed!");
        return false;
    }
    alcContext_ = alcCreateContext(alcDevice_, NULL);
    if (alcContext_ == nullptr) {
        spdlog::error("Couldn't create an OpenAL audio context!");
        return false;
    }
    alcMakeContextCurrent(alcContext_);

    /*AudioSource source = CreateAudioSource();
    source.BindBuffer(CreateAudioBuffer(LoadWAV("../../res/audio/music/intro.wav")));
    source.Play();*/

    return true;
}

AudioBuffer AudioPlayer::CreateAudioBuffer(const AudioBufferData& data) {
    ALuint bufferId;
    alGenBuffers(1, &bufferId);
    alBufferData(bufferId, data.alFormat, data.data, data.size, data.sampleRate);
    buffers_.push_back(bufferId);
    return AudioBuffer(bufferId);
}

AudioSource AudioPlayer::CreateAudioSource() {
    ALuint sourceId;
    alGenSources(1, &sourceId);
    sources_.push_back(sourceId);
    return AudioSource(sourceId);
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