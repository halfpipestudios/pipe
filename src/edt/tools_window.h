
#ifndef _TOOLS_WINDOW_H_
#define _TOOLS_WINDOW_H_

#include "editor_window.h"

struct ToolsWindow : EditorWindow {

    void Update(Editor *editor, f32 dt) override;
    void Render(Editor *editor) override;
    
};

#endif // _TOOLS_WINDOW_H_
