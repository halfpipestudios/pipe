#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "common.h"


struct File {
    void *data;
    u64 size;
};

struct Window {
    virtual ~Window() = default;
    virtual void Initialize(i32 w, i32 h) = 0;
    virtual void Terminate() = 0;    
    
    virtual void *GetOsWindow() = 0;
    virtual i32 GetWidth() = 0;
    virtual i32 GetHeight() = 0;

    virtual i32 GetPosX() = 0;
    virtual i32 GetPosY() = 0;
};

struct Input;

struct Platform {
    virtual ~Platform() = default;
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;
    
    // NOTE: window interface 
    virtual bool OnResize() = 0;
    virtual Window *GetWindow() = 0;
    virtual Input *GetInput() = 0;
    virtual void PollEvents() = 0;

    virtual f64 GetTimeInSeconds() = 0;
    
    // NOTE: memory interface
    virtual u64 GetPageSize() = 0;
    virtual void *MemoryReserve(u64 size) = 0;
    virtual void MemoryCommit(void *ptr, u64 size) = 0;
    virtual void *MemoryReserveAndCommit(u64 size) = 0;
    virtual void MemoryDecommit(void *ptr, u64 size) = 0;
    virtual void MemoryRelease(void *ptr, u64 size) = 0;

    virtual File ReadFileToStaticMemory(char *filepath) = 0;
    virtual File ReadFileToTemporalMemory(char *filepath) = 0;

    virtual void SetMousePosition(i32 x, i32 y) = 0;
    virtual void ShowMouse(bool value) = 0;

    virtual bool IsRunning() = 0;
};

#endif
