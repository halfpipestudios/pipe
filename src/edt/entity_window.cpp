#include "entity_window.h"
#include "editor.h"
#include "game.h"

void EntityWindow::Update(Editor *editor, f32 dt) {

    Level *level = editor->level;
    auto& entities = level->entities;

    _tgui_tree_view_begin(window, GetTreeviewId());
    _tgui_tree_view_root_node_begin("Entities", nullptr);

    for(i32 i = 0; i < entities.size; ++i) {
        SlotmapKey *entityKey = &entities[i];
        Entity_ *entity = EntityManager::Get()->GetEntity(*entityKey);
        _tgui_tree_view_node(entity->name, (void *)entityKey);
    }

    _tgui_tree_view_root_node_end();
    _tgui_tree_view_end((void **)&editor->selectedEntity);


}

void EntityWindow::Render(Editor *editor) {

}
