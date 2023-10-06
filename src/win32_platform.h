#ifndef _WIN32_PLATFORM_H_
#define _WIN32_PLATFORM_H_

#include <Windows.h>
#include <WindowsX.h>
#include <dwmapi.h>

#include "platform.h"
#include "input.h"

struct Win32Window : public Window {
    void Initialize(i32 w, i32 h) override;
    void Terminate() override;
    void *GetOsWindow() override;
    i32 GetWidth() override;
    i32 GetHeight() override;
    HWND hwnd;
    i32 width;
    i32 height;
};

struct Win32Platform : public Platform {
    void Initialize() override;
    void Terminate() override;
    
    // NOTE: window interface 
    Window *GetWindow() override;
    Input *GetInput() override;
    void PollEvents() override;
    bool IsRunning() override;

    // NOTE: memory interface
    u64 GetPageSize();
    void *MemoryReserve(u64 size);
    void MemoryCommit(void *ptr, u64 size);
    void *MemoryReserveAndCommit(u64 size);
    void MemoryDecommit(void *ptr, u64 size);
    void MemoryRelease(void *ptr, u64 size);

    Win32Window window;
    Input input;
    bool running;

private:
    SYSTEM_INFO systemInfo;
    void FatalError();

};

#endif
