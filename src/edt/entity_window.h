#ifndef _ENTITY_WINDOW_H_
#define _ENTITY_WINDOW_H_

#include "editor_window.h"

struct EntityWindow : EditorWindow {

    void Update(Editor *editor, f32 dt) override;
    void Render(Editor *editor) override;
    
    inline static char *GetTreeviewId() { return TGUI_ID; }

};

#endif // _ENTITY_WINDOW_H_
