#include "edt/editor_window.h"

void EditorWindow::Initialize(char *name, TGuiWindowFlags flags, EditorWindow *otherWindow, TGuiSplitDirection dir) {
    if(otherWindow == nullptr) {
        window = tgui_create_root_window(name, flags);
    } else {
        window = tgui_split_window(otherWindow->window, dir, name, flags);
    }
}

void EditorWindow::Terminate() {}
