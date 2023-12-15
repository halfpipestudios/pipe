#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <tgui.h>

#include "components_window.h"
#include "data_structures.h"
#include "entity_window.h"
#include "tools_window.h"
#include "game_window.h"
#include "camera.h"

struct Level;

struct Editor {
    
    void Initialize(Level *level);
    void Terminate();
    
    void Update(f32 dt);
    void Render();
    
    SlotmapKey *selectedEntity;
    bool paused;

    TGuiGfxBackend tguiBackend;

    GameWindow       gameWindow;
    ComponentsWindow compWindow;
    EntityWindow     entiWindow;
    ToolsWindow      toolWindow;

    Level *level;
    Camera camera;

};

#endif // _EDITOR_H_
