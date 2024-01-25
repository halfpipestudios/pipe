#ifndef _SOUND_H_
#define _SOUND_H_

#include "camera.h"
#include "algebra.h"
#include "asset_manager.h"

#include <xaudio2.h>
#include <x3daudio.h>

// NOTE: Sound manager -----------------------------------------------------------------

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
    
    virtual void Initialize(char *path);
    virtual void Deactivate(); 

    virtual void Play(bool loop, f32 vol);
    virtual void Pause();
    virtual void Stop();
    void SetVolume(float vol);
    
    Handle data;
    SoundChannel *channel;

};

struct Sound3D : Sound {
    
    virtual void Initialize(char *path) override;
    virtual void Deactivate() override;

    virtual void Play(bool loop, f32 vol) override;
    void Update(Vec3 pos_, Vec3 vel_);
    
    X3DAUDIO_EMITTER emitter;

    Sound3D *prev;
    Sound3D *next;
};

// NOTE: Sound system -----------------------------------------------------------------

struct SoundMixer {
    
    void Initialize();
    void Terminate();
    
    void Update(Camera *camera, Vec3 pos, Vec3 vel);

    void Play(Sound *sound, bool loop, f32 vol, bool is3d = false);
    void Stop(Sound *sound);

    IXAudio2* xaudio2;
    WAVEFORMATEX format;
    IXAudio2MasteringVoice* masterVoice;
    X3DAUDIO_HANDLE X3DInstance;

    SoundChannel idleChannelList;
    SoundChannel activeChannelList;

    X3DAUDIO_LISTENER listener;
    
    inline static SoundMixer *Get() { return &soundMixer; }

private:

    u32 reserveChannels;
    
    SoundChannel *AddSoundChannel();
    void RemoveSoundChannel(SoundChannel *channel);

    SoundMixer() {};
    static SoundMixer soundMixer;
};


#endif // _SOUND_H_
