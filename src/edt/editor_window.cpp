#include "edt/editor_window.h"

#include "input.h"
#include "platform_manager.h"

void EditorWindow::Initialize(char *name, TGuiWindowFlags flags, EditorWindow *otherWindow, TGuiSplitDirection dir) {
    if(otherWindow == nullptr) {
        window = tgui_create_root_window(name, flags);
    } else {
        window = tgui_split_window(otherWindow->window, dir, name, flags);
    }
}

void EditorWindow::Terminate() {}

i32 EditorWindow::GetWidth() {
    return tgui_window_width(window);
}

i32 EditorWindow::GetHeight() {
    return tgui_window_height(window);
}

i32 EditorWindow::GetMouseX() {
    Input *input = PlatformManager::Get()->GetInput();
    TGuiWindow *w = tgui_window_get_from_handle(window);
    i32 mouseX = CLAMP(input->state[0].mouseX - w->dim.min_x, 0, tgui_rect_width(w->dim)-1);
    return mouseX;
}

i32 EditorWindow::GetMouseY() {
    Input *input = PlatformManager::Get()->GetInput();
    TGuiWindow *w = tgui_window_get_from_handle(window);
    i32 mouseY = CLAMP(input->state[0].mouseY - w->dim.min_y, 0, tgui_rect_height(w->dim)-1);
    return mouseY;
}

i32 EditorWindow::GetPosX() {
    TGuiWindow *w = tgui_window_get_from_handle(window);
    return w->dim.min_x;
}

i32 EditorWindow::GetPosY() {
    TGuiWindow *w = tgui_window_get_from_handle(window);
    return w->dim.min_y;
}

bool EditorWindow::MouseIsHot() {
    
    Input *input = PlatformManager::Get()->GetInput();
    TGuiWindow *w = tgui_window_get_from_handle(window);

    if(!tgui_window_update_widget(w)) {
        return false;
    }

    if(tgui_window_is_grabbing()) {
        return false;
    }

    if(input->MouseX() >= w->dim.min_x &&
       input->MouseX() <= w->dim.max_x &&  
       input->MouseY() >= w->dim.min_y &&
       input->MouseY() <= w->dim.max_y) {
        return true;
    }
    return false;
}
