#include "xaudio2_sound_mixer.h"
#include "memory_manager.h"

#include <stdio.h>
#include <string.h>

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

static HRESULT LoadAudioFile(const char *path, WAVEFORMATEXTENSIBLE *wfx, XAUDIO2_BUFFER *buffer) {
    
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
    ReadChunkData(hFile, wfx, dwChunkSize, dwChunkPosition);
    
    // NOTE: Locate the DATA chuck
    FindChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition);
    BYTE * pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    // NOTE: Populate XAudio2 buffer
    buffer->AudioBytes = dwChunkSize;
    buffer->pAudioData = pDataBuffer;
    buffer->Flags = XAUDIO2_END_OF_STREAM;

    return S_OK;
}

/* -------------------------------------------------- */
/*     XAudio Playing Sound   implementation          */
/* -------------------------------------------------- */

static X3DAUDIO_HANDLE gX3DInstance;

static X3DAUDIO_DSP_SETTINGS GetDSPSettings() {
    static FLOAT32 monoMatrix[2];
    X3DAUDIO_DSP_SETTINGS settings = {};
    settings.SrcChannelCount = 1;
    settings.DstChannelCount = 2;
    settings.pMatrixCoefficients = (FLOAT32 *)monoMatrix;
    return settings;
}

static X3DAUDIO_EMITTER AudioEmiterFromSoundPosition(SoundPosition pos) {
    X3DAUDIO_EMITTER emitter = {};
    emitter.pCone               = nullptr;
    emitter.OrientFront         = {0, 0, -1};
    emitter.OrientTop           = {0, 1, 0};
    emitter.Position            = {pos.position.x, pos.position.y, pos.position.z};
    emitter.Velocity            = {pos.velocity.x, pos.velocity.y, pos.velocity.z};
    emitter.InnerRadius         = 1;
    emitter.InnerRadiusAngle    = 0;
    emitter.ChannelCount        = 1;
    emitter.ChannelRadius       = 0;
    emitter.pChannelAzimuths    = nullptr;
    emitter.pVolumeCurve        = nullptr;
    emitter.pLFECurve           = nullptr;
    emitter.pLPFDirectCurve     = nullptr;
    emitter.pLPFReverbCurve     = nullptr;
    emitter.pReverbCurve        = nullptr;
    emitter.CurveDistanceScaler = 1;
    emitter.DopplerScaler       = 1;
    return emitter;
}

static X3DAUDIO_LISTENER AudioListenerFromSoundPosition(SoundPosition pos) {
    X3DAUDIO_LISTENER listener = {};
    listener.OrientFront = {0, 0, 1};
    listener.OrientTop   = {0, 1, 0};
    listener.Position    = {pos.position.x, pos.position.y, pos.position.z};
    listener.Velocity    = {pos.velocity.x, pos.velocity.y, pos.velocity.z};
    listener.pCone       = nullptr;
    return listener;
}

void XAudio2PlayingSound::Play() {
    voice->Start(0);
}

void XAudio2PlayingSound::Play3DSound(SoundPosition listener, SoundPosition emiter) {
    Update3DSound(listener, emiter);
    Play();
}

void XAudio2PlayingSound::Update3DSound(SoundPosition listener, SoundPosition emiter) {
    X3DAUDIO_DSP_SETTINGS settings = GetDSPSettings();
    X3DAUDIO_LISTENER xaudioListener = AudioListenerFromSoundPosition(listener);
    X3DAUDIO_EMITTER xaudioEmitter = AudioEmiterFromSoundPosition(emiter);
    X3DAudioCalculate(gX3DInstance, &xaudioListener, &xaudioEmitter, X3DAUDIO_CALCULATE_MATRIX, &settings);
    voice->SetOutputMatrix(nullptr, 1, 2, settings.pMatrixCoefficients);
}

void XAudio2PlayingSound::Stop() {
    voice->Stop(0);
}

void XAudio2PlayingSound::IsPlaying() {
}

void XAudio2PlayingSound::SetVolume(f32 volume) {
    voice->SetVolume(volume);
}

/* -------------------------------------------------- */
/*     XAudio Sound Mixer implementation              */
/* -------------------------------------------------- */

void XAudio2SoundMixer::Initialize() {
    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr)) {
        printf("Cannot initialize xaudio2\n");
    }
    
    if(FAILED(hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        printf("Cannot create xaudio2 interface\n");
    }
    
    if(FAILED(hr = xaudio2->CreateMasteringVoice(&masterVoice))) {
        printf("Cannot create xaudio2 master voice\n");
    }

    printf("XAudio2 initialize perfectly\n");

    // NOTE: Allocate memory for all loaded sounds
    storedSoundBufferSize = 256;
    storedSoundBuffer = (XAudio2StoredSound *)MemoryManager::Get()->AllocStaticMemory(storedSoundBufferSize*sizeof(XAudio2StoredSound), 8);
    storedSoundBufferUsed = 0;

    // NOTE: Allocate memory for all playing sounds
    playingSoundPoolSize = 1024;
    playingSoundPool = (XAudio2PlayingSound *)MemoryManager::Get()->AllocStaticMemory(playingSoundPoolSize*sizeof(XAudio2PlayingSound), 8);
    
    // NOTE: Initialize free list of playing sounds
    for(u32 playingSoundIndex = 0; playingSoundIndex < playingSoundPoolSize; ++playingSoundIndex) {
        XAudio2PlayingSound *sound = playingSoundPool + playingSoundIndex;

        if(playingSoundIndex == (playingSoundPoolSize - 1)) {
            sound->next = nullptr;
        } else {
            sound->next = (playingSoundPool + (playingSoundIndex + 1));
        }
    }

    firstFreePlayingSoundList = playingSoundPool;

    // NOTE: Initialize 3d audio instance
    // TODO: Dont use a global for 3d audio
    DWORD dwChannelMask;       
    masterVoice->GetChannelMask(&dwChannelMask);
    X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, gX3DInstance);

}

void XAudio2SoundMixer::Terminate() {
    masterVoice->DestroyVoice();
    xaudio2->Release();
}

void XAudio2SoundMixer::LoadSound(char *path, char *name) {
    ASSERT(storedSoundBufferUsed < storedSoundBufferSize);
    XAudio2StoredSound *storeSound = storedSoundBuffer + storedSoundBufferUsed++;
    memcpy(storeSound->name, name, MIN(strlen(name), MAX_STORED_SOUND_NAME));
    if(FAILED(LoadAudioFile(path, &storeSound->wfx, &storeSound->buffer))) {
        printf("Cannot load audio file: %s\n", path);
    }

}

void XAudio2SoundMixer::UnloadSounds() {
    storedSoundBufferUsed = 0;
}

PlayingSound *XAudio2SoundMixer::AllocPlayingSound(char *name) {
    
    ASSERT(firstFreePlayingSoundList != nullptr);
    XAudio2PlayingSound *playingSound = firstFreePlayingSoundList;
    firstFreePlayingSoundList = firstFreePlayingSoundList->next;
    
    playingSound->sound = FindStoredSoundByName(name);
    ASSERT(playingSound->sound != nullptr);
    
    // TODO: Ensure that all voices use the same format so we can create and reuse them onece at initialization
    if(FAILED(xaudio2->CreateSourceVoice(&playingSound->voice, (WAVEFORMATEX*)&playingSound->sound->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, NULL, NULL))) {
        printf("Cannot create source voice for sound: %s\n", playingSound->sound->name);
    }
    
    if(FAILED(playingSound->voice->SubmitSourceBuffer(&playingSound->sound->buffer))) {
        printf("Cannot sumit sound buffer for: %s\n", playingSound->sound->name);
    }
    
    return playingSound;

}

void XAudio2SoundMixer::RemovePlayingSound(PlayingSound *sound) {
    
    // TODO: Ensure that all voices use the same format so we can create and reuse them onece at initialization
    XAudio2PlayingSound *xaudio2Sound = (XAudio2PlayingSound *)sound;
    xaudio2Sound->voice->DestroyVoice();
    xaudio2Sound->next = firstFreePlayingSoundList;
    firstFreePlayingSoundList = xaudio2Sound;

}

XAudio2StoredSound *XAudio2SoundMixer::FindStoredSoundByName(char *name) {
    for(u32 soundIndex = 0; soundIndex < storedSoundBufferUsed; ++soundIndex) {
        XAudio2StoredSound *sound = storedSoundBuffer + soundIndex;
        if(strcmp(sound->name, name) == 0) {
            return sound;
        }
    }
    return nullptr;
}
