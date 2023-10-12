#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"

#include <math.h>
#include <xaudio2.h>
#include <x3daudio.h>

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

static inline f32 to_rad(f32 degree) {
    return (degree / 180.0f) * 3.14159265359f;
}

typedef struct V3 {
    float x;
    float y;
    float z;
} V3;

static inline float v3_dot(V3 a, V3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline float v3_length_sqr(V3 a) {
    return v3_dot(a, a);
}

static inline float v3_length(V3 a) {
    return sqrtf(v3_length_sqr(a));
}

static inline V3 v3_rotate_y(V3 a, f32 angle) {
    f32 len = v3_length(a);
    V3 result = a;
    result.x = cosf(angle) * len;
    result.z = sinf(angle) * len;
    return result;
}

FLOAT32 monoMatrix[2];
V3 initialPosition;
f32 angle;

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition) {
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

HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset) {
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );
    DWORD dwRead;
    if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
        hr = HRESULT_FROM_WIN32( GetLastError() );
    return hr;
}

HRESULT LoadAudioFile(const char *path, WAVEFORMATEXTENSIBLE *wfx, XAUDIO2_BUFFER *buffer) {
    
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

int main() {
    
    /* ------------------------------ */
    /* XAudio2 TEST                   */
    /* ------------------------------ */
    
    // NOTE: Initialize COM
    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr))
        return hr;
    
    // NOTE: Create XAudio2Engine 
    IXAudio2* pXAudio2 = nullptr;
    if(FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        return hr;
    
    // NOTE: Create a Master voice to reproduce
    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if(FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
        return hr;

    printf("XAudio2 initialize perfectly\n");
    
    WAVEFORMATEXTENSIBLE ambient_wfx = {0};
    XAUDIO2_BUFFER ambient_buffer = {0};
    if(FAILED(hr = LoadAudioFile("./data/Fantasy Sound Library/Ambience_Cave_00.wav", &ambient_wfx, &ambient_buffer)))
        return hr;
    ambient_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    WAVEFORMATEXTENSIBLE spell_wfx = {0};
    XAUDIO2_BUFFER spell_buffer = {0};
    if(FAILED(hr = LoadAudioFile("./data/Fantasy Sound Library/Spell_00.wav", &spell_wfx, &spell_buffer)))
        return hr;
    spell_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    WAVEFORMATEXTENSIBLE trap_wfx = {0};
    XAUDIO2_BUFFER trap_buffer = {0};
    if(FAILED(hr = LoadAudioFile("./data/Fantasy Sound Library/Trap_00.wav", &trap_wfx, &trap_buffer)))
        return hr;

    // NOTE: Create and mix voice into mix voice
    IXAudio2SubmixVoice *mixVoice;
    pXAudio2->CreateSubmixVoice(&mixVoice, 1, 44100, 0, 0, 0, 0);

    XAUDIO2_SEND_DESCRIPTOR SFXSend = {0, mixVoice};
    XAUDIO2_VOICE_SENDS SFXSendList = {1, &SFXSend};

    // NOTE: Create a play source voice
    IXAudio2SourceVoice* ambientSourceVoice;
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&ambientSourceVoice, (WAVEFORMATEX*)&ambient_wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, NULL, NULL))) return hr;
    if(FAILED(hr = ambientSourceVoice->SubmitSourceBuffer(&ambient_buffer))) return hr;
    if(FAILED(hr = ambientSourceVoice->Start(0))) return hr;
    ambientSourceVoice->SetVolume(0.5f);

    // NOTE: Create a play source voice
    IXAudio2SourceVoice* spellSourceVoice;
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&spellSourceVoice, (WAVEFORMATEX*)&spell_wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, &SFXSendList, NULL))) return hr;
    spellSourceVoice->SubmitSourceBuffer(&spell_buffer);
    spellSourceVoice->Start(0);

    // NOTE: Create a play source voice
    IXAudio2SourceVoice* trapSourceVoice;
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&trapSourceVoice, (WAVEFORMATEX*)&trap_wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, &SFXSendList, NULL))) return hr;

    mixVoice->SetVolume(1);

    /* -------------------------------- */
    /* X3DAudio test (positional audio) */
    /* -------------------------------- */
    
    DWORD dwChannelMask;       
    pMasterVoice->GetChannelMask(&dwChannelMask);
    X3DAUDIO_HANDLE X3DInstance;
    X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);

    // NOTE: Audio Listener
    X3DAUDIO_LISTENER listener = {};
    listener.OrientFront = {0, 0, 1};
    listener.OrientTop   = {0, 1, 0};
    listener.Position    = {0, 0, 0};
    listener.Velocity    = {0, 0, 0};
    listener.pCone       = nullptr;
 
    X3DAUDIO_DSP_SETTINGS settings = {};
    settings.SrcChannelCount = 1;
    settings.DstChannelCount = 2;
    settings.pMatrixCoefficients = (FLOAT32 *)monoMatrix;

    // NOTE: Audio Emiter

    initialPosition = {10, 0, 0};
    angle = 0;

    X3DAUDIO_EMITTER emitter = {};
    emitter.pCone               = nullptr;
    emitter.OrientFront         = {0, 0, -1};
    emitter.OrientTop           = {0, 1, 0};
    emitter.Position            = {initialPosition.x, initialPosition.y, initialPosition.z};
    emitter.Velocity            = {0, 0, 0};
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

    /* ----------------------------------------- */

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();

    while(PlatformManager::Get()->IsRunning()) {
        
        f32 millisecondsPerFrame = 16;
        f32 secondsPerFrame = millisecondsPerFrame / 1000.0f;

        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();
        
        /* -------------------------------------------------------- */
        /*     X3DAudio Calculate and apply 3D audio to voices      */
        /* -------------------------------------------------------- */

        if(input->KeyJustPress(KEY_1)) {
            trapSourceVoice->Stop(0);
            trapSourceVoice->FlushSourceBuffers();
            trapSourceVoice->SubmitSourceBuffer(&trap_buffer);
            trapSourceVoice->Start(0);
        }

        // NOTE: Apply volumen and pitch
        V3 pos = v3_rotate_y(initialPosition, to_rad(angle));
        angle += 60 * secondsPerFrame;
        if(angle >= 360) angle = 0;

        emitter.Position = { pos.x, pos.y, pos.z };
        X3DAudioCalculate(X3DInstance, &listener, &emitter, X3DAUDIO_CALCULATE_MATRIX, &settings);
        mixVoice->SetOutputMatrix(pMasterVoice, 1, 2, settings.pMatrixCoefficients);

        /* -------------------------------------------------------- */

        Sleep(millisecondsPerFrame);
    }

    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
