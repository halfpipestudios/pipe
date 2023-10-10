#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"

#include "xaudio2.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

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
    
    // NOTE: Loading audio file
    
    WAVEFORMATEXTENSIBLE wfx = {0};
    XAUDIO2_BUFFER buffer = {0};
    HANDLE hFile = CreateFileA("./data/Lugia's Song (Original).wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if(INVALID_HANDLE_VALUE == hFile)
        return HRESULT_FROM_WIN32(GetLastError());

    if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32( GetLastError());
    
    printf("Loading audio file\n");

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    
    // NOTE: Locate the RIFF chuck
    FindChunk(hFile,fourccRIFF,dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile,&filetype,sizeof(DWORD),dwChunkPosition);
    if (filetype != fourccWAVE)
        return S_FALSE;

    // NOTE: Locate the FMT chuck
    FindChunk(hFile,fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
    
    // NOTE: Locate the DATA chuck
    FindChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition);
    BYTE * pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    // NOTE: Populate XAudio2 buffer
    buffer.AudioBytes = dwChunkSize;
    buffer.pAudioData = pDataBuffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    printf("Playing audio file\n");

    IXAudio2SourceVoice* pSourceVoice;
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx))) return hr;
    
    pSourceVoice->SetVolume(1);
    if(FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer))) return hr;
    if(FAILED(hr = pSourceVoice->Start(0))) return hr;

    /* ----------------------------------------- */

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();

    while(PlatformManager::Get()->IsRunning()) {
        
        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();

        Sleep(0.016);
    }

    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
