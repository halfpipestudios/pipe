#ifndef _XAUDIO2_SOUND_MIXER_H_
#define _XAUDIO2_SOUND_MIXER_H_

#include "sound_mixer.h"

#define MAX_STORED_SOUND_NAME 256

struct XAudio2StoredSound {
    char name[MAX_STORED_SOUND_NAME];
    void *buffer;
};

struct XAudio2PlayingSound : public PlayingSound {

    void Play(f32 volume, i32 count);
    void Play3DSound(SoundPosition listener, SoundPosition emiter);
    void Update3DSound(SoundPosition listener, SoundPosition emiter);
    
    void Stop();

    void IsPlaying();
    void SetVolume();

private:

};

struct XAudio2SoundMixer : public SoundMixer {
    
    void Initialize();
    void Terminate();

    void LoadSound(char *path);

    PlayingSound *AllocPlayingSound(char *name);
    void RemovePlayingSound(PlayingSound *sound);

private:
    
    u32 playingSoundPoolSize;
    XAudio2PlayingSound *playingSoundPool;
    XAudio2PlayingSound *firstFreePlayingSoundList;

    u32 storedSoundBufferSize;
    XAudio2StoredSound *storedSoundBuffer;

};

#endif
