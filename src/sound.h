#ifndef _SOUND_H_
#define _SOUND_H_

#include "asset_manager.h"
#include <xaudio2.h>

// NOTE: Sound manager -----------------------------------------------------------------

struct SoundData {
    WAVEFORMATEXTENSIBLE wfx;
    XAUDIO2_BUFFER buffer;
};

struct SoundManager : AssetManager<SoundData> {

    void Load(SoundData *data, const char *name) override;
    void Unload(SoundData *data) override;

    inline static SoundManager *Get() { return &soundManager; }
    static SoundManager soundManager; 

};

struct SoundChannel {
    IXAudio2SourceVoice* voice;

    SoundChannel *prev;
    SoundChannel *next;

    void Initialize();
    void Terminate();
};

struct Sound {
    SoundData    *data;
    SoundChannel *channel;

    // TODO: Function to play stop and set volume of sound HERE!

};

// NOTE: Sound system -----------------------------------------------------------------

struct SoundMixer {
    
    void Initialize();
    void Terminate();

    void Play(Sound *sound);
    void Stop(Sound *sound);

    IXAudio2* xaudio2;
    WAVEFORMATEX format;
    IXAudio2MasteringVoice* masterVoice;

    SoundChannel *idleChannelList;
    SoundChannel *activeChannelList;
    
    inline static SoundMixer *Get() { return &soundMixer; }

private:

    u32 reserveChannels;
    
    SoundChannel *AddSoundChannel();
    void RemoveSoundChannel(SoundChannel *channel);

    SoundMixer() {};
    static SoundMixer soundMixer;
};


#endif // _SOUND_H_
