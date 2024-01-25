#include "sound.h"
#include "memory_manager.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

static HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition) {
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK) {
        DWORD dwRead;
        if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        switch (dwChunkType) {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
                hr = HRESULT_FROM_WIN32( GetLastError() );
            break;

        default:
            if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dwChunkDataSize, NULL, FILE_CURRENT ) )
            return HRESULT_FROM_WIN32( GetLastError() );            
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc) {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;

}

static HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset) {
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );
    DWORD dwRead;
    if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
        hr = HRESULT_FROM_WIN32( GetLastError() );
    return hr;
}

static HRESULT LoadAudioFile(const char *path, SoundData *soundData) {
    
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if(INVALID_HANDLE_VALUE == hFile)
        return HRESULT_FROM_WIN32(GetLastError());

    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32( GetLastError());

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    
    // NOTE: Locate the RIFF chuck
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
    if (filetype != fourccWAVE)
        return S_FALSE;

    // NOTE: Locate the FMT chuck
    FindChunk(hFile,fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &soundData->format, dwChunkSize, dwChunkPosition);
    
    WAVEFORMATEX expectedFormat = SoundMixer::Get()->format; 
    ASSERT(soundData->format.Format.wFormatTag      == expectedFormat.wFormatTag);
    ASSERT(soundData->format.Format.nChannels       == expectedFormat.nChannels);
    ASSERT(soundData->format.Format.nSamplesPerSec  == expectedFormat.nSamplesPerSec);
    ASSERT(soundData->format.Format.wBitsPerSample  == expectedFormat.wBitsPerSample);
    ASSERT(soundData->format.Format.nBlockAlign     == expectedFormat.nBlockAlign);
    ASSERT(soundData->format.Format.nAvgBytesPerSec == expectedFormat.nAvgBytesPerSec);
    
    // NOTE: Locate the DATA chuck
    FindChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition);
    BYTE * pDataBuffer = (BYTE *)MemoryManager::Get()->AllocStaticMemory(dwChunkSize, 8);
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    // NOTE: Populate XAudio2 buffer
    soundData->size = dwChunkSize;
    soundData->data = (void *)pDataBuffer;

    CloseHandle(hFile);

    return S_OK;
}



// NOTE: Sound manager -----------------------------------------------------------------

SoundManager SoundManager::soundManager;

void SoundManager::Load(SoundData *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/sounds/", name); 
    LoadAudioFile(path, data);

    printf("Sound: %s loaded\n", path);
}

void SoundManager::Unload(SoundData *data) {
    printf("remove sound: %llu\n", (u64)data);
}


// NOTE: playing sound for sound system ------------------------------------------------

void ChannelCallback::OnBufferEnd(void *context) {
    SoundChannel *channel = (SoundChannel *)context;
    channel->Desactivate();
}

void SoundChannel::Initialize() {

    memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
    buffer.Flags = XAUDIO2_END_OF_STREAM;	
    buffer.pContext = this;

    static ChannelCallback channelCallback;
    if(FAILED(SoundMixer::Get()->xaudio2->CreateSourceVoice(&voice, (WAVEFORMATEX*)&SoundMixer::Get()->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &channelCallback, NULL, NULL))) {
        ASSERT(!"Error initializing source voice");
    }

    next = nullptr;
    prev = nullptr; 
}

void SoundChannel::Terminate() {
    voice->DestroyVoice();
}

void SoundChannel::Activate(Sound *sound_, bool loop) {
    ListRemove(this);
    ListInsertBack(&SoundMixer::Get()->activeChannelList, this);
    sound = sound_;
    sound->channel = this;
    SoundData *data = SoundManager::Get()->Dereference(sound->data);
    buffer.AudioBytes = data->size;
    buffer.pAudioData = (const BYTE *)data->data;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
}

void SoundChannel::Desactivate() {
    sound->Deactivate();
    sound = nullptr;
    buffer.AudioBytes = 0;
    buffer.pAudioData = nullptr;
    buffer.LoopCount = 0;
    ListRemove(this);
    ListInsertBack(&SoundMixer::Get()->idleChannelList, this);
    
    static float volumes[2] = {1.0f, 1.0f};
    voice->SetChannelVolumes(2, volumes);
}

// NOTE: Sound system -----------------------------------------------------------------

SoundMixer SoundMixer::soundMixer;

SoundChannel *SoundMixer::AddSoundChannel() {
    SoundChannel *channel = (SoundChannel *)MemoryManager::Get()->AllocStaticMemory(sizeof(SoundChannel), 8);
    channel->Initialize();
    ListInsertBack(&idleChannelList, channel);
    return channel;
}

void SoundMixer::RemoveSoundChannel(SoundChannel *channel) {
    channel->Terminate();
    ListRemove(channel);
}

void SoundMixer::Initialize() {
    
    // NOTE: Initialize sound manager
    SoundManager::Get()->Initialize(256);

    
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
    
    ListInit(&idleChannelList);
    ListInit(&activeChannelList);

    reserveChannels = 64;

    for(u32 i = 0; i < reserveChannels; ++i) {
        AddSoundChannel();
    }

    // NOTE: Setup 3d sound system

    DWORD channelMask;
    masterVoice->GetChannelMask(&channelMask);

    X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);
    memset(&listener, 0, sizeof(X3DAUDIO_LISTENER));

}

void SoundMixer::Terminate() {
    
    // NOTE: Terminate all source channels

    SoundChannel *channel = nullptr;
    
    channel = ListGetTop(&idleChannelList);
    while(!ListIsEnd(&idleChannelList, channel)) {
        SoundChannel *toFree = channel;
        channel = channel->next;
        RemoveSoundChannel(toFree);
    }
    
    channel = ListGetTop(&activeChannelList);
    while(!ListIsEnd(&activeChannelList, channel)) {
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

    // NOTE: Terminate sound manager
    SoundManager::Get()->Terminate();
}

void SoundMixer::Update(Camera *camera, Vec3 pos, Vec3 vel) {
    Vec3 nFront = camera->front.Normalized();
    Vec3 nUp = camera->up.Normalized();

    listener.OrientFront = {nFront.x, nFront.y, nFront.z};
    listener.OrientTop   = {nUp.x, nUp.y, nUp.z};
    listener.Position    = {pos.x, pos.y, pos.z};
    listener.Velocity    = {vel.x, vel.y, vel.z};
}

void SoundMixer::Play(Sound *sound, bool loop, f32 vol, bool is3d) {
    SoundChannel *channel = ListGetTop(&idleChannelList);
    channel->Activate(sound, loop);

    if(FAILED(channel->voice->SubmitSourceBuffer(&channel->buffer))) {
        ASSERT(!"Cannot submit sound data to channel");
    }
    sound->SetVolume(vol);
    if(is3d) {
        static float volumes[2] = {};
        channel->voice->SetChannelVolumes(2, volumes);
    }
    channel->voice->Start(0);
}

void SoundMixer::Stop(Sound *sound) {
    
    if(!sound->channel) return;
    ASSERT(sound->channel);
    sound->channel->voice->Stop(0);
    if(FAILED(sound->channel->voice->FlushSourceBuffers())) {
        ASSERT(!"Cannot flush channel");
    }
    
    sound->channel->Desactivate();
}

// Sound interface -------------------------------------------------------

void Sound::Initialize(char *path) {
    data = SoundManager::Get()->GetAsset(path);
    channel = nullptr;
}

void Sound::Deactivate() {
    channel = nullptr;
}

void Sound::Sound::Play(bool loop, f32 vol) {
    SoundMixer::Get()->Play(this, loop, vol);
}

void Sound::Pause() {
    ASSERT(!"Uninplemented function!");
}

void Sound::Stop() {
    SoundMixer::Get()->Stop(this);
}

void Sound::SetVolume(float vol) {
    if(channel) channel->voice->SetVolume(vol);
}

// 3D Sound interface -------------------------------------------------------

void Sound3D::Initialize(char *path) {
    Sound::Initialize(path);
    emitter.ChannelCount = 1;
    emitter.CurveDistanceScaler = 1.0f;
    emitter.DopplerScaler = 1.0f;
}

void Sound3D::Deactivate() {
    ListRemove(this);
    Sound::Deactivate();
}

void Sound3D::Update(Vec3 pos_, Vec3 vel_) {
    // NOTE: Setup emmiter
    
    emitter.OrientFront = {0, 0, 1};
    emitter.OrientTop   = {0, 1, 0};
    emitter.Position    = {pos_.x, pos_.y, pos_.z};
    emitter.Velocity    = {vel_.x, vel_.y, vel_.z};
    
    // NOTE: Calculate Sound Channel volumes 
    
    MemoryManager::Get()->BeginTemporalMemory();

    X3DAUDIO_DSP_SETTINGS DSPSettings = {};
    DSPSettings.SrcChannelCount = 1;
    DSPSettings.DstChannelCount = 2;
    DSPSettings.pMatrixCoefficients = (FLOAT32 *)MemoryManager::Get()->AllocTemporalMemory(sizeof(FLOAT32)*DSPSettings.DstChannelCount, 8);

    X3DAudioCalculate(SoundMixer::Get()->X3DInstance, &SoundMixer::Get()->listener, &emitter,
                      X3DAUDIO_CALCULATE_MATRIX|X3DAUDIO_CALCULATE_DOPPLER|X3DAUDIO_CALCULATE_LPF_DIRECT|X3DAUDIO_CALCULATE_REVERB,
                      &DSPSettings);

    channel->voice->SetChannelVolumes(DSPSettings.DstChannelCount, DSPSettings.pMatrixCoefficients);
    channel->voice->SetFrequencyRatio(DSPSettings.DopplerFactor);

    MemoryManager::Get()->EndTemporalMemory();

}

void Sound3D::Play(bool loop, f32 vol) {
    SoundMixer::Get()->Play(this, loop, 1.0f, true);
}
