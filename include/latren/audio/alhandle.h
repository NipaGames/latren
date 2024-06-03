#pragma once

#include <AL/al.h>
#include <AL/alc.h>

// we'll use these intermediate handles, let's try to keep openal code away from the programmer
class OpenAlHandle {
protected:
    ALuint handle_;
public:
    OpenAlHandle() : handle_(AL_NONE) { }
    OpenAlHandle(ALuint handle) : handle_(handle) { }
    ALuint GetALHandle() const { return handle_; }
    bool IsNull() const { return handle_ == AL_NONE; }
    void Reset() { handle_ = AL_NONE; }
};

typedef OpenAlHandle AudioBufferHandle;