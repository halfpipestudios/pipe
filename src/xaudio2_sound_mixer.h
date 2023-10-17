#ifndef _XAUDIO2_SOUND_MIXER_H_
#define _XAUDIO2_SOUND_MIXER_H_

#include "sound_mixer.h"

#include <xaudio2.h>
#include <x3daudio.h>

#define MAX_STORED_SOUND_NAME 256

struct XAudio2StoredSound {
    char name[MAX_STORED_SOUND_NAME];
    WAVEFORMATEXTENSIBLE wfx;
    XAUDIO2_BUFFER buffer;
};

struct XAudio2PlayingSound : public PlayingSound {

    void Play(f32 volume, i32 count);
    void Play3DSound(SoundPosition listener, SoundPosition emiter);
    void Update3DSound(SoundPosition listener, SoundPosition emiter);
    
    void Stop();

    void IsPlaying();
    void SetVolume();

    XAudio2StoredSound *sound;
    IXAudio2SourceVoice* voice;

    XAudio2PlayingSound *next;
};

struct XAudio2SoundMixer : public SoundMixer {
    
    void Initialize();
    void Terminate();

    void LoadSound(char *path, char *name);
    void UnloadSounds();

    PlayingSound *AllocPlayingSound(char *name);
    void RemovePlayingSound(PlayingSound *sound);

private:
    
    u32 playingSoundPoolSize;
    XAudio2PlayingSound *playingSoundPool;
    XAudio2PlayingSound *firstFreePlayingSoundList;

    u32 storedSoundBufferSize;
    XAudio2StoredSound *storedSoundBuffer;
    u32 storedSoundBufferUsed;
 
    IXAudio2* xaudio2;
    IXAudio2MasteringVoice* masterVoice;

    XAudio2StoredSound *FindStoredSoundByName(char *name);

};

#endif
