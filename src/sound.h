#ifndef _SOUND_H_
#define _SOUND_H_

#include "asset_manager.h"
#include <xaudio2.h>

// NOTE: Sound manager -----------------------------------------------------------------
//

struct Sound;

struct SoundData {
    WAVEFORMATEXTENSIBLE format;
    void *data;
    u32 size;
};

struct SoundManager : AssetManager<SoundData> {

    void Load(SoundData *data, const char *name) override;
    void Unload(SoundData *data) override;

    inline static SoundManager *Get() { return &soundManager; }
    static SoundManager soundManager; 

};

struct ChannelCallback : IXAudio2VoiceCallback {

    void OnStreamEnd() override {}
    void OnVoiceProcessingPassEnd() override {}
    void OnVoiceProcessingPassStart(u32 samplreRequire) override {}

    void OnBufferEnd(void *context) override;
    void OnBufferStart(void *context) override {}
    
    void OnLoopEnd(void *context) override {}
    void OnVoiceError(void *context, HRESULT error) override {}

};


struct SoundChannel {

    void Initialize();
    void Terminate();

    void Activate(Sound *sound_, bool loop);
    void Desactivate();
    
    IXAudio2SourceVoice* voice;
    XAUDIO2_BUFFER buffer;
    Sound *sound;

    SoundChannel *prev;
    SoundChannel *next;
};

struct Sound {
    
    void Initialize(char *path);

    void Play(bool loop);
    void Pause();
    void Stop();

    Handle data;
    SoundChannel *channel;
};

// NOTE: Sound system -----------------------------------------------------------------

struct SoundMixer {
    
    void Initialize();
    void Terminate();

    void Play(Sound *sound, bool loop);
    void Stop(Sound *sound);

    IXAudio2* xaudio2;
    WAVEFORMATEX format;
    IXAudio2MasteringVoice* masterVoice;

    SoundChannel idleChannelList;
    SoundChannel activeChannelList;
    
    inline static SoundMixer *Get() { return &soundMixer; }

private:

    u32 reserveChannels;
    
    SoundChannel *AddSoundChannel();
    void RemoveSoundChannel(SoundChannel *channel);

    SoundMixer() {};
    static SoundMixer soundMixer;
};


#endif // _SOUND_H_
