#include "tgui_docker.h"

#include "tgui_painter.h"
#include "tgui.h"

#include <stdio.h>

extern TGui state;
extern TGuiInput input;
TGuiDocker docker;

/* ------------------------------------
          Internal implementation 
   ------------------------------------ */

TGuiDockerNode *node_alloc(void) {
    TGuiDockerNode *node = NULL;

    if(docker.first_free_node) {
        node = docker.first_free_node;
        docker.first_free_node = docker.first_free_node->next;
    } else {
        node = tgui_arena_push_struct(&state.arena, TGuiDockerNode, 8);
    }

    TGUI_ASSERT(node != NULL);
    memset(node, 0, sizeof(TGuiDockerNode));
    
    return node;
}

TGuiDockerNode *root_node_alloc(TGuiDockerNode *parent) {
    TGUI_ASSERT(!parent || parent->type == TGUI_DOCKER_NODE_ROOT);
    
    TGuiDockerNode *node  = node_alloc();
    TGuiDockerNode *dummy = node_alloc();
    
    tgui_clink_list_init(dummy);
     
    node->type = TGUI_DOCKER_NODE_ROOT;
    node->childs = dummy;
    node->parent = parent;
    node->dir = TGUI_SPLIT_DIR_NONE;

    return node;
}

TGuiDockerNode *window_node_alloc(TGuiDockerNode *parent) {
    
    TGuiDockerNode *node = node_alloc();

    node->type = TGUI_DOCKER_NODE_WINDOW;
    node->parent = parent;

    node->dummy_allocated_window = tgui_allocated_window_node_alloc();
    node->windows = &node->dummy_allocated_window->window;
    tgui_clink_list_init(node->windows);
    node->windows_count = 0;

    return node;
}

TGuiDockerNode *split_node_alloc(TGuiDockerNode *parent, tgui_f32 position) {
    TGUI_ASSERT(parent && parent->type == TGUI_DOCKER_NODE_ROOT);
    
    TGuiDockerNode *node = node_alloc();
    
    node->type = TGUI_DOCKER_NODE_SPLIT;
    node->parent = parent;
    node->split_position = position;

    return node;
}

void node_free(TGuiDockerNode *node) {
    
    if(node->type == TGUI_DOCKER_NODE_ROOT) {
        TGuiDockerNode *dummy = node->childs;

        TGuiDockerNode *child = node->childs->next;
        while(!tgui_clink_list_end(child, node->childs)) {
            TGuiDockerNode *to_free = child;
            child = child->next;
            node_free(to_free);
        }
        
        dummy->next = docker.first_free_node;
        docker.first_free_node = dummy;
    } else if(node->type == TGUI_DOCKER_NODE_WINDOW) {
        tgui_allocated_window_node_free(node->dummy_allocated_window);
    }

    node->next = docker.first_free_node;
    docker.first_free_node = node;

}

static inline tgui_b32 position_overlap_node(TGuiDockerNode *node, tgui_s32 x, tgui_s32 y) {
    TGuiRectangle dim = node->dim;
    return tgui_rect_point_overlaps(dim, x, y);
}

static TGuiDockerNode *get_node_in_position(TGuiDockerNode *node, tgui_s32 x, tgui_s32 y) {
    
    if(position_overlap_node(node, x, y)) {
        switch (node->type) {
        
        case TGUI_DOCKER_NODE_ROOT: {
            TGuiDockerNode *child = node->childs->next;
            while(!tgui_clink_list_end(child, node->childs)) {
                TGuiDockerNode *result = get_node_in_position(child, x, y);
                child = child->next; 
                if(result) return result;
            }
        } break;
        case TGUI_DOCKER_NODE_SPLIT: { 
            return node;
        } break;
        case TGUI_DOCKER_NODE_WINDOW: { 
            return node; 
        } break;
        
        }
    
    }

    return 0;
}

char *tgui_docker_node_to_str(TGuiDockerNodeType type) {
    switch (type) {
    case TGUI_DOCKER_NODE_ROOT:   { return "TGUI_DOCKER_NODE_ROOT"; } break;
    case TGUI_DOCKER_NODE_SPLIT:  { return "TGUI_DOCKER_NODE_SPLIT"; } break;
    case TGUI_DOCKER_NODE_WINDOW: { return "TGUI_DOCKER_NODE_WINDOW"; } break;
    }
    TGUI_ASSERT(!"Invalid code path");
    return NULL;
}

#if 0
static void tgui_node_print(TGuiDockerNode *node) {
    switch (node->type) {
    case TGUI_DOCKER_NODE_ROOT:   { printf("TGUI_DOCKER_NODE_ROOT\n");   } break;
    case TGUI_DOCKER_NODE_SPLIT:  { printf("TGUI_DOCKER_NODE_SPLIT\n");  } break;
    case TGUI_DOCKER_NODE_WINDOW: { printf("TGUI_DOCKER_NODE_WINDOW\n"); } break;
    }
}
#endif

static inline void calculate_mouse_rel_x_y(TGuiDockerNode *split, TGuiDockerNode *parent, tgui_f32 *x, tgui_f32 *y) {
    
    tgui_f32 epsilon = 0.05;

    tgui_f32 clamp_max = 1.0f;
    tgui_f32 clamp_min = 0.0f;
    
    if(split->next->next->type == TGUI_DOCKER_NODE_SPLIT) clamp_max = split->next->next->split_position;
    if(split->prev->prev->type == TGUI_DOCKER_NODE_SPLIT) clamp_min = split->prev->prev->split_position;
    
    clamp_max -= epsilon;
    clamp_min += epsilon;

    TGuiRectangle parent_dim = parent->dim;
    *x = TGUI_CLAMP((tgui_f32)(input.mouse_x - parent_dim.min_x) / (tgui_f32)tgui_rect_width(parent_dim) , clamp_min, clamp_max);
    *y = TGUI_CLAMP((tgui_f32)(input.mouse_y - parent_dim.min_y) / (tgui_f32)tgui_rect_height(parent_dim), clamp_min, clamp_max);

}

static TGuiRectangle calculate_menu_bar_rect(TGuiDockerNode *window) {
    TGUI_ASSERT(window->type == TGUI_DOCKER_NODE_WINDOW);

    TGuiRectangle menu_bar_rect = window->dim;
    menu_bar_rect.min_x += 1;
    menu_bar_rect.max_x -= 1;
    menu_bar_rect.min_y += 1;
    menu_bar_rect.max_y = menu_bar_rect.min_y + MENU_BAR_HEIGHT - 1;

    return menu_bar_rect;
}

static inline tgui_u32 get_widow_index(TGuiWindow *window) {
    TGuiDockerNode *window_node = window->parent;
    tgui_u32 window_index = 0;
    TGuiWindow *other = window_node->windows->next;
    while(window != other) { 
        ++window_index;
        other = other->next;
    }
    return window_index;
}

static TGuiRectangle calculate_window_tab_rect(TGuiWindow *window) {
    TGuiDockerNode *window_node = window->parent;
    
    TGuiRectangle result = calculate_menu_bar_rect(window_node);
    tgui_u32 window_index = get_widow_index(window);
    
    tgui_s32 width = tgui_rect_width(result) / window_node->windows_count;
    result.min_x += window_index * width;
    result.max_x = result.min_x + width - 1;

    return result;
}

static tgui_b32 mouse_in_menu_bar(TGuiDockerNode *window) {
    TGUI_ASSERT(window->type == TGUI_DOCKER_NODE_WINDOW);

    tgui_s32 x = input.mouse_x;
    tgui_s32 y = input.mouse_y;
    TGuiRectangle dim = calculate_menu_bar_rect(window);

    return (dim.min_x <= x && x <= dim.max_x && dim.min_y <= y && y <= dim.max_y);
}

static void split_node_move(TGuiDockerNode *node) {
    TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_SPLIT);
    TGUI_ASSERT(node->parent != NULL && node->parent->type == TGUI_DOCKER_NODE_ROOT);

    TGuiDockerNode *parent = node->parent;
    
    tgui_f32 mouse_rel_x, mouse_rel_y;
    calculate_mouse_rel_x_y(node, parent, &mouse_rel_x, &mouse_rel_y);

    if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {
        node->split_position = mouse_rel_x;
    } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {
        node->split_position = mouse_rel_y;
    }
    
    tgui_docker_node_recalculate_dim(node->prev);
    tgui_docker_node_recalculate_dim(node->next);

    if(!input.mouse_button_is_down) {
        docker.active_node = NULL;
    }

}

static void calculate_drop_split_dir_and_pos(TGuiDockerNode *mouse_over_node, TGuiSplitDirection *dir, TGuiPosition *pos) {
    TGuiRectangle dim = mouse_over_node->dim;

    tgui_u32 l_distance = input.mouse_x - dim.min_x;
    tgui_u32 r_distance = dim.max_x - input.mouse_x;
    
    tgui_u32 t_distance = input.mouse_y - dim.min_y;
    tgui_u32 b_distance = dim.max_y - input.mouse_y;
    
    tgui_u32 min_h_disntace = 0;
    TGuiPosition h_pos = TGUI_POS_FRONT;

    if(l_distance < r_distance) {
        min_h_disntace = l_distance;
        h_pos = TGUI_POS_BACK;
    } else {
        min_h_disntace = r_distance;
        h_pos = TGUI_POS_FRONT;
    }

    tgui_u32 min_v_disntace = 0;
    TGuiPosition v_pos = TGUI_POS_FRONT;

    if(t_distance < b_distance) {
        min_v_disntace = t_distance;
        v_pos = TGUI_POS_BACK;
    } else {
        min_v_disntace = b_distance;
        v_pos = TGUI_POS_FRONT;
    }

    if(min_h_disntace < min_v_disntace) {
        *dir = TGUI_SPLIT_DIR_VERTICAL;
        *pos = h_pos;
    } else {
        *dir = TGUI_SPLIT_DIR_HORIZONTAL;
        *pos = v_pos;
    }
}

static TGuiDockerNode *root_node_get_first_window(TGuiDockerNode *root) {
    TGUI_ASSERT(root->type == TGUI_DOCKER_NODE_ROOT);
    TGuiDockerNode *child = root->childs->next;
    if(child->type == TGUI_DOCKER_NODE_WINDOW) {
        return child;
    } else if(child->type == TGUI_DOCKER_NODE_ROOT) {
        return root_node_get_first_window(child);
    } else {
        TGUI_ASSERT(!"Split node cannot be the first child of root node");
        return root;
    }
}

static TGuiDockerNode *split_node_get_first_window(TGuiDockerNode *split) {
    TGUI_ASSERT(split->type == TGUI_DOCKER_NODE_SPLIT);
    TGuiDockerNode *window = split->next; 
    /* TODO: Find the best way to handle where to place the window when mouse is over a split node */
    if(window->type == TGUI_DOCKER_NODE_ROOT) {
        window = root_node_get_first_window(window);
    } 
    TGUI_ASSERT(window->type == TGUI_DOCKER_NODE_WINDOW);
    return window;
}

static void calculate_preview_split(TGuiDockerNode *mouse_over_node) {
        
        TGuiDockerNode *node = mouse_over_node;

        if(mouse_over_node->type == TGUI_DOCKER_NODE_WINDOW && mouse_in_menu_bar(mouse_over_node)) {
            TGuiRectangle menu_bar_rect = calculate_menu_bar_rect(mouse_over_node);
            docker.preview_window = menu_bar_rect;
            return; 
        }

        TGuiSplitDirection dir;
        TGuiPosition pos;
        calculate_drop_split_dir_and_pos(mouse_over_node, &dir, &pos);
        
        if(node->type == TGUI_DOCKER_NODE_SPLIT) {
            node = split_node_get_first_window(mouse_over_node);
            dir = TGUI_SPLIT_DIR_HORIZONTAL;
        }

        TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_WINDOW || node->type == TGUI_DOCKER_NODE_ROOT);
        
        docker.preview_window = node->dim;
        TGUI_ASSERT(dir != TGUI_SPLIT_DIR_NONE);
        
        if(dir == TGUI_SPLIT_DIR_VERTICAL) {
            
            if(pos == TGUI_POS_BACK) {
                docker.preview_window.max_x = (node->dim.max_x + node->dim.min_x) / 2; 
            } else {
                docker.preview_window.min_x = (node->dim.max_x + node->dim.min_x) / 2; 
            }
        
        } else if(dir == TGUI_SPLIT_DIR_HORIZONTAL) {
            if(pos == TGUI_POS_BACK) {
                docker.preview_window.max_y = (node->dim.max_y + node->dim.min_y) / 2; 
            } else {
                docker.preview_window.min_y = (node->dim.max_y + node->dim.min_y) / 2; 
            } 
        
        }

}

TGuiRectangle tgui_docker_get_client_rect(TGuiDockerNode *window) {
    TGUI_ASSERT(window->type == TGUI_DOCKER_NODE_WINDOW);
    
    if(tgui_rect_invalid(window->dim)) {
        return window->dim;
    }

    TGuiRectangle client_rect = window->dim;
    client_rect.min_x += 1;
    client_rect.min_y += (MENU_BAR_HEIGHT + 1);
    client_rect.max_x -= 1;
    client_rect.max_y -= 1;
    return client_rect;
}

static TGuiWindow *get_window_from_tab(TGuiDockerNode *window_node, tgui_u32 pos_x) {
    TGuiRectangle menu_bar_rect = calculate_menu_bar_rect(window_node);
    tgui_u32 width = tgui_rect_width(menu_bar_rect) / window_node->windows_count;

    tgui_u32 index = TGUI_CLAMP((tgui_s32)((pos_x - menu_bar_rect.min_x) / width), (tgui_s32)0, (tgui_s32)window_node->windows_count - 1);
    
    tgui_u32 current_index = 0;
    TGuiWindow *window = window_node->windows->next;
    while(!tgui_clink_list_end(window, window_node->windows) && (current_index < index)) {
        window = window->next;
        current_index += 1;
    }
    
    return window;
}

void tgui_docker_window_node_add_window(TGuiDockerNode *window_node, struct TGuiWindow *window) { 
    tgui_clink_list_insert_back(window_node->windows, window)
    window_node->windows_count += 1;

    window->parent = window_node;
}


void tgui_docker_window_node_add_window_at_mouse_pos(TGuiDockerNode *window_node, struct TGuiWindow *window) { 
    TGuiWindow *window_at_mouse = get_window_from_tab(window_node, input.mouse_x);
    tgui_clink_list_insert_back(window_at_mouse, window)
    window_node->windows_count += 1;

    window->parent = window_node;
}

void tgui_docker_window_node_remove_window(struct TGuiWindow *window) {
    TGuiDockerNode *window_node = window->parent;
    TGUI_ASSERT(window_node);
    window_node->windows_count -= 1;
    tgui_clink_list_remove(window);
    window->parent = NULL;
}

tgui_b32 tgui_docker_window_has_tabs(TGuiDockerNode *window_node) {
    return window_node->windows_count > 1;
}

static void window_grabbing_preview(void) {
    TGuiDockerNode *mouse_over_node = get_node_in_position(docker.root, input.mouse_x, input.mouse_y);
    if(mouse_over_node) {
        calculate_preview_split(mouse_over_node);
    }
}

static TGuiWindow *tgui_window_node_get_active_window(TGuiDockerNode *window_node) {
    TGUI_ASSERT(window_node->type == TGUI_DOCKER_NODE_WINDOW);
    return tgui_window_get_from_handle(window_node->active_window);
}

static void window_grabbing_start(TGuiDockerNode *window) {
    
    TGuiDockerNode *parent = window->parent;

    docker.grabbing_window = true;
    docker.grabbing_window_start = false;

    if(tgui_docker_window_has_tabs(window)) {
        
        TGuiWindow *w = get_window_from_tab(window, docker.saved_mouse_x);
        TGuiWindow *active_window = tgui_window_node_get_active_window(window);
        if(active_window == w) {
            if(w->prev == window->windows) {
                window->active_window = w->prev->prev->id;
            } else {
                window->active_window = w->prev->id;
            }
        }

        tgui_docker_window_node_remove_window(w);
        
        TGuiDockerNode *new_window_node = window_node_alloc(0);
        tgui_docker_window_node_add_window(new_window_node, w);
        docker.active_node = new_window_node;

    } else {

        if(!parent) return;

        TGuiDockerNode *split = NULL;

        if(tgui_clink_list_first(window, parent->childs) && tgui_clink_list_last(window, parent->childs)) {
            TGUI_ASSERT(!"Root nodes cannot have only one child");
        } else if(tgui_clink_list_first(window, parent->childs)) {
            split = window->next;
        } else if(tgui_clink_list_last(window, parent->childs)) {
            split = window->prev;
        } else {
            split = window->next;
        }

        TGUI_ASSERT(split != NULL && split->type == TGUI_DOCKER_NODE_SPLIT);
        tgui_clink_list_remove(window);
        tgui_clink_list_remove(split);
        node_free(split);
        
        docker.active_node = window;

        /* NOTE: If the parent is left with just one child, we need to replace the parent node with the only child */
        TGuiDockerNode *child = parent->childs->next;
        if(tgui_clink_list_first(child, parent->childs) && tgui_clink_list_last(child, parent->childs)) {
            
            tgui_clink_list_remove(child);
            
            if(parent == docker.root) {
                tgui_docker_set_root_node(child);
                node_free(parent);
                parent = child;
            } else {
                child->parent = parent->parent;

                tgui_clink_list_insert_front(parent, child);
                tgui_clink_list_remove(parent);
                node_free(parent);
                parent = child->parent;
            }
        }

        tgui_docker_node_recalculate_dim(parent);
    }
    
    TGUI_ASSERT(docker.active_node->type == TGUI_DOCKER_NODE_WINDOW);
    docker.active_node->dim = tgui_rect_set_invalid();

}

static void window_grabbing_end(TGuiDockerNode *node) {

    if(!docker.grabbing_window) return;

    TGuiDockerNode *mouse_over_node = get_node_in_position(docker.root, input.mouse_x, input.mouse_y);
    if(mouse_over_node) {
        if(mouse_over_node->type == TGUI_DOCKER_NODE_WINDOW && mouse_in_menu_bar(mouse_over_node)) {
            
            TGuiWindow *window = node->windows->next;
            while(!tgui_clink_list_end(window, node->windows)) {
                TGuiWindow *to_move = window;
                window = window->next;
                tgui_docker_window_node_add_window_at_mouse_pos(mouse_over_node, to_move);
                mouse_over_node->active_window = to_move->id;
            }
             
            node_free(node);

        } else {
            TGuiSplitDirection dir;
            TGuiPosition pos;
            calculate_drop_split_dir_and_pos(mouse_over_node, &dir, &pos);

            if(mouse_over_node->type == TGUI_DOCKER_NODE_SPLIT) {
                mouse_over_node = split_node_get_first_window(mouse_over_node);
                dir = TGUI_SPLIT_DIR_HORIZONTAL;
            }
            
            tgui_docker_node_split(mouse_over_node, dir, pos, node);
            tgui_docker_node_recalculate_dim(node->parent);

        }
    }

    docker.active_node = NULL;
    docker.grabbing_window = false;
    docker.preview_window = (TGuiRectangle){0};
}

static void set_cursor_state(TGuiDockerNode *mouse_over) {
    
    /* TODO: Should probably handle the situation when we are performing an action i a better way */
    if(docker.active_node) return;

    state.cursor = TGUI_CURSOR_ARROW;

    
    if(!mouse_over) {
        state.cursor = TGUI_CURSOR_ARROW;
        return;
    }

    switch (mouse_over->type) {
    
    case TGUI_DOCKER_NODE_ROOT: {
        state.cursor = TGUI_CURSOR_ARROW;
    } break; 
    case TGUI_DOCKER_NODE_WINDOW: {
        if(mouse_in_menu_bar(mouse_over)) {
            state.cursor = TGUI_CURSOR_HAND;
        } else {
            state.cursor = TGUI_CURSOR_ARROW;
        }
    } break; 
    case TGUI_DOCKER_NODE_SPLIT: {
        TGUI_ASSERT(mouse_over->parent);
        TGuiDockerNode *parent = mouse_over->parent;
        if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {
            state.cursor = TGUI_CURSOR_H_ARROW;
        } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {
            state.cursor = TGUI_CURSOR_V_ARROW;
        }
    } break; 
    
    }

}

static void split_recalculate_dim(TGuiDockerNode *split) {
    TGUI_ASSERT(split->type == TGUI_DOCKER_NODE_SPLIT);
    TGUI_ASSERT(split->parent && split->parent->type == TGUI_DOCKER_NODE_ROOT);


    TGuiDockerNode *parent = split->parent;
    TGUI_ASSERT(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL || parent->dir == TGUI_SPLIT_DIR_VERTICAL);
    
    TGuiRectangle dim = parent->dim;
    split->dim = dim;
    
    if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {
        
        tgui_s32 abs_split_position = dim.min_x + (split->split_position * tgui_rect_width(dim));
        split->dim.min_x =  abs_split_position - SPLIT_HALF_SIZE;
        split->dim.max_x =  abs_split_position + SPLIT_HALF_SIZE;
    
    } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {

        tgui_s32 abs_split_position = dim.min_y + (split->split_position * tgui_rect_height(dim));
        split->dim.min_y =  abs_split_position - SPLIT_HALF_SIZE;
        split->dim.max_y =  abs_split_position + SPLIT_HALF_SIZE;
    }
     
}

/* ------------------------------------
        Termporal drawing methods
   ------------------------------------ */

void node_draw(TGuiPainter *painter, TGuiDockerNode *node) {
    if(!node) return;

    switch (node->type) {
     
    case TGUI_DOCKER_NODE_WINDOW: {
        TGuiWindow *window = tgui_window_get_from_handle(node->active_window);
        if(!tgui_window_flag_is_set(window, TGUI_WINDOW_TRANSPARENT)) {
            tgui_painter_draw_rectangle(painter, node->dim, 0x777777);
        }
        
        tgui_u32 border_color = 0x333333;
        tgui_painter_draw_rectangle_outline(painter, node->dim, border_color);

        tgui_u32 menu_bar_color = 0x333333;
        TGuiRectangle menu_bar_rect = calculate_menu_bar_rect(node);
        tgui_painter_draw_rectangle(painter, menu_bar_rect, menu_bar_color);
        

        TGuiRectangle saved_clip = painter->clip;
        painter->clip = tgui_rect_intersection(painter->clip, menu_bar_rect);
        
        if(!tgui_docker_window_has_tabs(node)) {
            
            TGuiWindow *window = node->windows->next;

            char *label = window->name;
            TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);
            
            tgui_s32 label_x = menu_bar_rect.min_x + tgui_rect_width(menu_bar_rect) / 2 - tgui_rect_width(label_rect) / 2;
            tgui_s32 label_y = menu_bar_rect.min_y + tgui_rect_height(menu_bar_rect) / 2 - tgui_rect_height(label_rect) / 2;
            tgui_font_draw_text(painter, label_x, label_y, label, strlen(label), 0xffffff);

        } else {
            TGuiWindow *window = node->windows->next;
            while(!tgui_clink_list_end(window, node->windows)) {
                
                TGuiRectangle tab_rect = calculate_window_tab_rect(window);
                tgui_painter_draw_rectangle_outline(painter, tab_rect, 0x444444);
                
                TGuiRectangle saved_clip2 = painter->clip;
                painter->clip = tgui_rect_intersection(painter->clip, tab_rect);

                char * label = window->name;
                TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);
                tgui_s32 label_x = tab_rect.min_x + tgui_rect_width(tab_rect) / 2 - tgui_rect_width(label_rect) / 2;
                tgui_s32 label_y = tab_rect.min_y + tgui_rect_height(tab_rect) / 2 - tgui_rect_height(label_rect) / 2;
                tgui_font_draw_text(painter, label_x, label_y, label, strlen(label), 0xffffff);
                
                if(window->id == node->active_window) {
                    TGuiWindow *active_window = tgui_window_node_get_active_window(node);
                    tgui_painter_draw_rectangle_outline(painter, calculate_window_tab_rect(active_window), 0xaaaaff);
                }
                
                painter->clip = saved_clip2;

                window = window->next;
            }
        }
        
        painter->clip = saved_clip;

    } break;

    case TGUI_DOCKER_NODE_SPLIT: {
        tgui_painter_draw_rectangle(painter, node->dim, 0x444444);
    } break;

    case TGUI_DOCKER_NODE_ROOT: {
        TGuiDockerNode * child = node->childs->next;
        while (!tgui_clink_list_end(child, node->childs)) { 
            node_draw(painter, child);
            child = child->next;
        }
    } break;

    }
}

void tgui_docker_root_node_draw(TGuiPainter *painter) {
    node_draw(painter, docker.root);
}

void tgui_docker_draw_preview(TGuiPainter *painter) {
    if(docker.grabbing_window) {
        tgui_u32 border_color = 0xaaaaff;
        tgui_painter_draw_rectangle_outline(painter, docker.preview_window, border_color);
    }
}

/* --------------------------
         TGUI Docker API 
   -------------------------- */

void tgui_docker_initialize(void) {
    memset(&docker, 0, sizeof(TGuiDocker));
    
    docker.root = NULL;
    docker.first_free_node = NULL;
    docker.grabbing_window = false;
    docker.preview_window = (TGuiRectangle){0};
}

void tgui_docker_terminate(void) {
    if(docker.root != NULL) {
        node_free(docker.root);
    }
}

void docker_node_print(TGuiDockerNode *node) {
    switch(node->type) {
    case TGUI_DOCKER_NODE_ROOT:   { printf("Root Node\n"); } break;
    case TGUI_DOCKER_NODE_WINDOW: { printf("Window Node\n"); } break;
    case TGUI_DOCKER_NODE_SPLIT:  { printf("Split Node\n"); } break;

    }
}

static void docker_update_active_node(void) {

    if(docker.active_node) return;
    
    TGuiDockerNode *mouse_over_node = get_node_in_position(docker.root, input.mouse_x, input.mouse_y);
    if(!mouse_over_node) return;

    set_cursor_state(mouse_over_node);

    if(mouse_over_node->type == TGUI_DOCKER_NODE_SPLIT) {
        if(input.mouse_button_is_down && !input.mouse_button_was_down) {
            docker.active_node = mouse_over_node;
        }
    }

    if(mouse_over_node->type == TGUI_DOCKER_NODE_WINDOW) {
        if(mouse_in_menu_bar(mouse_over_node) && input.mouse_button_is_down && !input.mouse_button_was_down) {

            docker.active_node = mouse_over_node;
            
            if(!tgui_docker_window_has_tabs(mouse_over_node)) {
                docker.grabbing_window_start = true;
                
                if(mouse_over_node == docker.root) {
                    docker.grabbing_window_start = false;
                    docker.active_node = NULL;
                }
            
            } else {
                docker.saved_mouse_x = input.mouse_x;
                docker.saved_mouse_y = input.mouse_y;
            }
        }
    }
}

void tgui_docker_update(void) {
    
    if(!docker.root) return;

    if(input.window_resize) {
        tgui_docker_node_recalculate_dim(docker.root);
        input.window_resize = false;
    }

    docker_update_active_node();

    if(docker.active_node && docker.active_node->type == TGUI_DOCKER_NODE_SPLIT) {
        split_node_move(docker.active_node); 
    
    } 

    if(docker.active_node && docker.active_node->type == TGUI_DOCKER_NODE_WINDOW) {

        TGuiWindow *w = get_window_from_tab(docker.active_node, input.mouse_x);
        docker.active_node->active_window = w->id;

        if(!docker.grabbing_window_start && !docker.grabbing_window) {
            if(!input.mouse_button_is_down) {
                docker.active_node = NULL;
            } else {
                tgui_s32 dx = ((tgui_s32)input.mouse_x - (tgui_s32)docker.saved_mouse_x);
                tgui_s32 dy = ((tgui_s32)input.mouse_y - (tgui_s32)docker.saved_mouse_y);
                tgui_u32 distance_sqr = dx*dx + dy*dy;
                
                if(distance_sqr >= 32*32) {
                    docker.grabbing_window_start = true;
                }

            }
        }

        if(docker.grabbing_window_start) {
            window_grabbing_start(docker.active_node);
        }
        
        if(docker.grabbing_window) {
            window_grabbing_preview();
            
            if(!input.mouse_button_is_down) {
                window_grabbing_end(docker.active_node);
            }
        }
    } 
}

void tgui_docker_set_root_node(TGuiDockerNode *node) {
    TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_ROOT || node->type == TGUI_DOCKER_NODE_WINDOW);
    docker.root = node;
    node->parent = NULL;
}

void tgui_docker_root_set_child(TGuiDockerNode *root, TGuiDockerNode *node) {
    TGUI_ASSERT(root->type == TGUI_DOCKER_NODE_ROOT);
    TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_WINDOW || node->type == TGUI_DOCKER_NODE_ROOT);
    
    TGUI_ASSERT(tgui_clink_list_is_empty(root->childs));
    if(tgui_clink_list_is_empty(root->childs)) {
        node->parent = root;
        tgui_clink_list_insert_front(root->childs, node);
    }

}

tgui_b32 tgui_docker_window_is_visible(TGuiDockerNode *window_node, TGuiWindow *window) {
    if(docker.grabbing_window && docker.active_node == window_node) {
        return false;
    }
    
    if(window_node->active_window != window->id) {
        return false;
    }

    return true;
}

void tgui_docker_node_split(TGuiDockerNode *node, TGuiSplitDirection dir, TGuiPosition pos, TGuiDockerNode *node1) {
    TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_WINDOW || node->type == TGUI_DOCKER_NODE_ROOT);
    
    TGuiDockerNode *parent = node->parent;
    
    /* NOTE: The only case this can happend is if we have a window as the root */
    if(!parent) {
        TGUI_ASSERT(docker.root == node);
        TGuiDockerNode *root = root_node_alloc(0);
        tgui_docker_set_root_node(root);
        tgui_docker_root_set_child(root, node);
        parent = root;
    }

    TGUI_ASSERT(parent && parent->type == TGUI_DOCKER_NODE_ROOT);
    
    if(parent->dir == TGUI_SPLIT_DIR_NONE) {
        parent->dir = dir;
    } else if(parent->dir != dir) {
        
        TGuiDockerNode *root = root_node_alloc(parent);
        root->dir = dir;
        tgui_clink_list_insert_back(node, root);
        tgui_clink_list_remove(node);
        tgui_docker_root_set_child(root, node);
        parent = root;
    
    }

    node1->parent = parent;

    tgui_f32 split_position = 0.0f;
    
    if(tgui_clink_list_first(node, parent->childs) && tgui_clink_list_last(node, parent->childs)) {
        split_position = 0.5f;
    } else if(tgui_clink_list_first(node, parent->childs)) {
    
        TGuiDockerNode *split_next = node->next;
        TGUI_ASSERT(split_next->type == TGUI_DOCKER_NODE_SPLIT);
        split_position = split_next->split_position * 0.5f;
    
    } else if(tgui_clink_list_last(node, parent->childs)) {
    
        TGuiDockerNode *split_prev = node->prev;
        TGUI_ASSERT(split_prev->type == TGUI_DOCKER_NODE_SPLIT);
        split_position = split_prev->split_position + ((1 - split_prev->split_position) * 0.5f);
    
    } else {
        
        TGuiDockerNode *split_next = node->next;
        TGuiDockerNode *split_prev = node->prev;
        TGUI_ASSERT(split_next->type == TGUI_DOCKER_NODE_SPLIT);
        TGUI_ASSERT(split_prev->type == TGUI_DOCKER_NODE_SPLIT);
        split_position = split_prev->split_position + ((split_next->split_position - split_prev->split_position) * 0.5f);
    
    }

    TGUI_ASSERT(split_position >= 0.0f && split_position <= 1.0f);

    TGuiDockerNode *split = split_node_alloc(node->parent, split_position);
    if(pos == TGUI_POS_FRONT) {
        tgui_clink_list_insert_front(node, split);
        tgui_clink_list_insert_front(split, node1);
    } else {
        tgui_clink_list_insert_back(node, split);
        tgui_clink_list_insert_back(split, node1);
    }
    

}

void tgui_docker_node_recalculate_dim(TGuiDockerNode *node) {

    TGUI_ASSERT(node->type == TGUI_DOCKER_NODE_WINDOW || node->type == TGUI_DOCKER_NODE_ROOT);

    TGuiDockerNode *parent = node->parent;
    
    if(parent) {
       
        node->dim = parent->dim;

        TGUI_ASSERT(parent->type == TGUI_DOCKER_NODE_ROOT);

        if(tgui_clink_list_first(node, parent->childs) && tgui_clink_list_last(node, parent->childs)) {
            
            TGUI_ASSERT(!"Root nodes cannot hace only one child");
         
        } else if(tgui_clink_list_first(node, parent->childs)) {
            TGUI_ASSERT(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL || parent->dir == TGUI_SPLIT_DIR_VERTICAL);
            
            TGuiDockerNode *split_next = node->next;
            TGUI_ASSERT(split_next->type == TGUI_DOCKER_NODE_SPLIT);
            split_recalculate_dim(split_next);

            if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {
                
                node->dim.min_x = parent->dim.min_x;
                node->dim.max_x = split_next->dim.min_x - 1;
            
            } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {

                node->dim.min_y = parent->dim.min_y;
                node->dim.max_y = split_next->dim.min_y - 1;
            }

        } else if(tgui_clink_list_last(node, parent->childs)) {
            TGUI_ASSERT(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL || parent->dir == TGUI_SPLIT_DIR_VERTICAL);

            TGuiDockerNode *split_prev = node->prev;
            TGUI_ASSERT(split_prev->type == TGUI_DOCKER_NODE_SPLIT);
            split_recalculate_dim(split_prev);

            if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {
            
                node->dim.min_x = split_prev->dim.max_x + 1; 
                node->dim.max_x = parent->dim.max_x;

            } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {

                node->dim.min_y = split_prev->dim.max_y + 1; 
                node->dim.max_y = parent->dim.max_y;
            }
        
        } else {
            TGUI_ASSERT(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL || parent->dir == TGUI_SPLIT_DIR_VERTICAL);

            TGuiDockerNode *split_next = node->next;
            TGuiDockerNode *split_prev = node->prev;
            TGUI_ASSERT(split_next->type == TGUI_DOCKER_NODE_SPLIT);
            TGUI_ASSERT(split_prev->type == TGUI_DOCKER_NODE_SPLIT);
            split_recalculate_dim(split_next);
            split_recalculate_dim(split_prev);

            if(parent->dir == TGUI_SPLIT_DIR_VERTICAL) {

                node->dim.min_x = split_prev->dim.max_x + 1; 
                node->dim.max_x = split_next->dim.min_x - 1;

            } else if(parent->dir == TGUI_SPLIT_DIR_HORIZONTAL) {

                node->dim.min_y = split_prev->dim.max_y + 1; 
                node->dim.max_y = split_next->dim.min_y - 1;
            }
        }
    } else {
        node->dim = (TGuiRectangle){0, 0, (tgui_s32)input.resize_w - 1, (tgui_s32)input.resize_h - 1};
    }

    if(node->type == TGUI_DOCKER_NODE_ROOT) {
       
        TGuiDockerNode *child = node->childs->next;
        while (!tgui_clink_list_end(child, node->childs)) { 

            /* NOTE: For now split node dim are calculated online when needed */
            /* TODO: Maybe add dirty flag to actually use the cashed dimension */
            if(child->type == TGUI_DOCKER_NODE_SPLIT) {
                child = child->next;
            }
            
            tgui_docker_node_recalculate_dim(child);

            child = child->next;
        }

    }

}
