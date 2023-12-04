#ifndef _SOUND_MIXER_H_
#define _SOUND_MIXER_H_

#include "common.h"
#include "math.h"

#define MAX_SOUND_NAME 256

struct SoundPosition {
    Vec3 position;
    Vec3 velocity;
};

struct PlayingSound {
    virtual ~PlayingSound() = default; 
    virtual void Play() = 0;
    virtual void Play3DSound(SoundPosition listener, SoundPosition emiter) = 0;
    virtual void Update3DSound(SoundPosition listener, SoundPosition emiter) = 0;
    
    virtual void Stop() = 0;

    virtual void IsPlaying() = 0;
    virtual void SetVolume(f32 volume) = 0;
};

struct SoundMixer {
    virtual ~SoundMixer() = default;
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;
    
    virtual void LoadSound(char *path, char name) = 0;
    virtual void UnloadSounds() = 0;

    virtual PlayingSound *AllocPlayingSound(char *name) = 0;
    virtual void RemovePlayingSound(PlayingSound *sound) = 0;
};

#endif
