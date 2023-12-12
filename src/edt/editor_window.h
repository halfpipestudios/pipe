#ifndef _EDITOR_WINDOW_H_
#define _EDITOR_WINDOW_H_

#include "common.h"
#include <tgui.h>

struct Editor;

struct EditorWindow {
    
    virtual void Initialize(char *name, TGuiWindowFlags flags, EditorWindow *otherWindow, TGuiSplitDirection dir);
    virtual void Terminate();
    
    virtual void Update(Editor *editor, f32 dt) = 0;
    virtual void Render(Editor *editor) = 0;

    TGuiWindowHandle window; 
};

#endif // _EDITOR_WINDOW_H_
