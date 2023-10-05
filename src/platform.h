#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "common.h"

struct Window {
    virtual void Initialize(i32 w, i32 h) = 0;
    virtual void Terminate() = 0;    
    virtual void *GetOsWindow() = 0;
    virtual i32 GetWidth() = 0;
    virtual i32 GetHeight() = 0;

};

struct Input;

struct Platform {
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;
    virtual Window *GetWindow() = 0;
    virtual Input *GetInput() = 0;
    virtual void PollEvents() = 0;
    virtual bool IsRunning() = 0;
};

#endif
