#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <tgui.h>

#include "components_window.h"
#include "entity_window.h"
#include "tools_window.h"
#include "game_window.h"

struct Game;
struct Level;
struct Entity_;

struct Editor {
    
    void Initialize(Game *game);
    void Terminate();
    
    void Update(f32 dt);
    void Render();
    
    Game *game;
    Entity_ *selectedEntity;
    bool paused;

    TGuiGfxBackend tguiBackend;

    GameWindow       gameWindow;
    ComponentsWindow compWindow;
    EntityWindow     entiWindow;
    ToolsWindow      toolWindow;
};

#endif // _EDITOR_H_
