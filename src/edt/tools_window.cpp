#include "tools_window.h"
#include "editor.h"

void ToolsWindow::Update(Editor *editor, f32 dt) {

    u32 buttonW = 120;
    u32 buttonH = 30;

    i32 w = tgui_window_width(window); 
    i32 h = tgui_window_height(window); 
    i32 x = w / 2 - buttonW / 2; if(x < 0) x = 0;
    i32 y = 10;
    tgui_button(window, "Save Level", x, y);
    tgui_button(window, "Load Level", x, y +  30 + 10);
    tgui_button(window, "Add Entity", x, y +  60 + 20);
    if(tgui_button(window, "Remove Entity", x, y +  90 + 30)) {
        /*
        if(selectedEntity) {
            game->level.em.DeleteEntity(selectedEntity); 
            selectedEntity = nullptr;
            _tgui_tree_view_deselect(EntityWindow::GetTreeviewId());

        }
        */
    }

    char *gameStateButton[] = {
        "Pause Game",
        "Play Game",
    };

    if(tgui_button(window, gameStateButton[editor->paused], x, y + 120 + 40)) {
        editor->paused = !editor->paused;
    }

}

void ToolsWindow::Render(Editor *editor) {

}
