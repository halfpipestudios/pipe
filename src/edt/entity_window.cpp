#include "entity_window.h"
#include "editor.h"
#include "game.h"

void EntityWindow::Update(Editor *editor, f32 dt) {

    Level *level = &editor->game->level;
    auto& entities = level->em.GetEntities();

    _tgui_tree_view_begin(window, GetTreeviewId());
    _tgui_tree_view_root_node_begin("Entities", nullptr);

    for(i32 i = 0; i < entities.size; ++i) {
        Entity_ *entity = &entities[i];
        _tgui_tree_view_node(entity->name, (void *)entity);
    }

    _tgui_tree_view_root_node_end();
    _tgui_tree_view_end((void **)&editor->selectedEntity);


}

void EntityWindow::Render(Editor *editor) {

}
