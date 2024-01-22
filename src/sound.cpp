#include "sound.h"
#include "memory_manager.h"

// NOTE: Sound manager -----------------------------------------------------------------

SoundManager SoundManager::soundManager;

void SoundManager::Load(SoundData *data, const char *name) {
}

void SoundManager::Unload(SoundData *data) {
}


// NOTE: playing sound for sound system ------------------------------------------------

void SoundChannel::Initialize() {
    
    if(FAILED(SoundMixer::Get()->xaudio2->CreateSourceVoice(&voice, (WAVEFORMATEX*)&SoundMixer::Get()->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, NULL, NULL))) {
        ASSERT(!"Error initializing source voice");
    }

    next = nullptr;
    prev = nullptr; 
}

void SoundChannel::Terminate() {
    voice->DestroyVoice();

    next = nullptr;
    prev = nullptr; 
}

// NOTE: Sound system -----------------------------------------------------------------

SoundMixer SoundMixer::soundMixer;

SoundChannel *SoundMixer::AddSoundChannel() {
    SoundChannel *channel = (SoundChannel *)MemoryManager::Get()->AllocStaticMemory(sizeof(SoundChannel), 8);
    channel->Initialize();
    ListInsertBack(idleChannelList, channel);
    return channel;
}

void SoundMixer::RemoveSoundChannel(SoundChannel *channel) {
    channel->Terminate();
    ListRemove(channel);
}

void SoundMixer::Initialize() {
    
    HRESULT hr;

    // NOTE: Initialize COM object
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr)) {
        ASSERT(!"Cannot initialize xaudio2");
    }

    // NOTE: Initialize xaudio 2 
    
    if(FAILED(hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        ASSERT(!"Cannot create xaudio2 interface");
    }
    
    if(FAILED(hr = xaudio2->CreateMasteringVoice(&masterVoice))) {
        ASSERT(!"Cannot create xaudio2 master voice");
    }

    // NOTE: Setup audio buffers format
    format.wFormatTag      = WAVE_FORMAT_PCM;
    format.nChannels       = 2;
    format.nSamplesPerSec  = 44100;
    format.wBitsPerSample  = 16;
    format.nBlockAlign     = (format.nChannels * format.wBitsPerSample) >> 3;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize          = 0;
    
    // NOTE: Setup audio buffers lists  
    
    ListInit(idleChannelList);
    ListInit(activeChannelList);

    reserveChannels = 64;

    for(u32 i = 0; i < reserveChannels; ++i) {
        AddSoundChannel();
    }

}

void SoundMixer::Terminate() {
    
    // NOTE: Terminate all source channels

    SoundChannel *channel = nullptr;
    
    channel = ListGetTop(idleChannelList);
    while(!ListIsEnd(idleChannelList, channel)) {
        SoundChannel *toFree = channel;
        channel = channel->next;
        RemoveSoundChannel(toFree);
    }
    
    channel = ListGetTop(activeChannelList);
    while(!ListIsEnd(activeChannelList, channel)) {
        SoundChannel *toFree = channel;
        channel = channel->next;
        RemoveSoundChannel(toFree);
    }

    // NOTE: Terminate xaudi2 master voice
    masterVoice->DestroyVoice();
    
    // NOTE: Terminate xaudio2
    xaudio2->Release();
    
    // NOTE: Terminate COM object
    CoUninitialize();

}

void SoundMixer::Play(Sound *sound) {
    ASSERT(sound->channel == nullptr);
    sound->channel = ListGetTop(idleChannelList);
    ListRemove(sound->channel);
    ListInsertBack(activeChannelList, sound->channel);

    if(FAILED(sound->channel->voice->SubmitSourceBuffer(&sound->data->buffer))) {
        ASSERT(!"Cannot submit sound data to channel");
    }
    sound->channel->voice->Start(0);
}

void SoundMixer::Stop(Sound *sound) {
    ASSERT(sound->channel);
    sound->channel->voice->Stop(0);
    if(FAILED(sound->channel->voice->FlushSourceBuffers())) {
        ASSERT(!"Cannot flush channel");
    }

    ListRemove(sound->channel);
    ListInsertBack(idleChannelList, sound->channel);
    sound->channel = nullptr;
}
