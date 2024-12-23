#include "tgui_painter.h"
#include "tgui_docker.h"

#include "tgui.h"
#include "tgui_gfx.h"
#include "tgui_serializer.h"
#include "tgui_os.h"

#include <stdio.h>

/* -------------------------- */
/*       Hash Function     */
/* -------------------------- */
 static tgui_u64 murmur_hash64A(const void * key, int len, tgui_u64 seed) {
        const tgui_u64 m = 0xc6a4a7935bd1e995ull;
        const int r = 47;
        tgui_u64 h = seed ^ (len * m);
        
        const tgui_u64 * data = (const tgui_u64 *)key;
        const tgui_u64 * end = data + (len/8);
        
        while(data != end) {
            tgui_u64 k = *data++;

            k *= m; 
            k ^= k >> r; 
            k *= m; 
    
            h ^= k;
            h *= m; 
        }

        const unsigned char * data2 = (const unsigned char*)data;

        switch(len & 7) {
            case 7: h ^= (tgui_u64)(data2[6]) << 48; 
            case 6: h ^= (tgui_u64)(data2[5]) << 40;
            case 5: h ^= (tgui_u64)(data2[4]) << 32;
            case 4: h ^= (tgui_u64)(data2[3]) << 24;
            case 3: h ^= (tgui_u64)(data2[2]) << 16;
            case 2: h ^= (tgui_u64)(data2[1]) << 8;
            case 1: h ^= (tgui_u64)(data2[0]);
            h *= m;
        };
        
        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
} 

tgui_u64 murmur_hash64A(const void * key, int len, tgui_u64 seed);

/* TODO: This global variables should be static */

TGui state;
TGuiInput input;
TGuiFont font;
extern TGuiDocker docker;


TGuiInput *tgui_get_input(void) {
    return &input;
}

TGuiCursor tgui_get_cursor_state(void) {
    return state.cursor;
}

tgui_u64 tgui_hash(void *bytes, tgui_u64 size) {
    return murmur_hash64A(bytes, size, 123);
}

/* ---------------------- */
/*       TGui Font        */
/* ---------------------- */

static tgui_u32 get_codepoint_index(tgui_u32 codepoint) {
    
    if((codepoint < font.glyph_rage_start) || (codepoint > font.glyph_rage_end)) {
        codepoint = (tgui_u32)'?';
    }

    tgui_u32 index = (codepoint - font.glyph_rage_start);
    return index;
}

void tgui_font_initilize(TGuiArena *arena) {
    
    struct TGuiOsFont *os_font = tgui_os_font_create(arena, "./data/fonts/LiberationMono-Regular.ttf", 18);
    //struct OsFont *os_font = os_font_create(arena, "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf", 18);
    
    font.glyph_rage_start = 32;
    font.glyph_rage_end = 126;
    font.glyph_count = (font.glyph_rage_end - font.glyph_rage_start + 1);
    font.glyphs = tgui_arena_push_array(arena, TGuiGlyph, font.glyph_count, 8);
    tgui_os_font_get_vmetrics(os_font, &font.ascent, &font.descent, &font.line_gap);

    void *temp_buffer = tgui_arena_alloc(arena, 512*512, 8);

    for(tgui_u32 glyph_index = font.glyph_rage_start; glyph_index <= font.glyph_rage_end; ++glyph_index) {
        
        tgui_s32 w, h, bpp;
        tgui_os_font_rasterize_glyph(os_font, glyph_index, &temp_buffer, &w, &h, &bpp);
    
        TGuiGlyph *glyph = font.glyphs + (glyph_index - font.glyph_rage_start);
        glyph->bitmap.pixels = tgui_arena_alloc(arena, sizeof(tgui_u32)*w*h, 8);
        glyph->bitmap.width  = w;
        glyph->bitmap.height = h;
        
        memset(glyph->bitmap.pixels, 0, sizeof(tgui_u32)*w*h);
        
        tgui_u8 *src_row  = temp_buffer;
        tgui_u32 *des_row = glyph->bitmap.pixels;
        for(tgui_u32 y = 0; y < (tgui_u32)h; ++y) {
            tgui_u8 *src  = src_row;
            tgui_u32 *des = des_row;
            for(tgui_u32 x = 0; x < (tgui_u32)w; ++x) { 
                *des++ = 0x00ffffff | (*src++ << 24); 
            }
            src_row += w;
            des_row += w;
        }

        tgui_os_font_get_glyph_metrics(os_font, glyph_index, &glyph->adv_width, &glyph->left_bearing, &glyph->top_bearing);

        tgui_texture_atlas_add_bitmap(state.default_texture_atlas, &glyph->bitmap);
    }
    
    font.font = os_font;

    TGuiGlyph *default_glyph = font.glyphs + get_codepoint_index(' '); 
    font.max_glyph_width  = default_glyph->adv_width;
    font.max_glyph_height = font.ascent - font.descent + font.line_gap;

}

void tgui_font_terminate(void) {
    tgui_os_font_destroy(font.font);
}

TGuiGlyph *tgui_font_get_codepoint_glyph(tgui_u32 codepoint) {
    return font.glyphs + get_codepoint_index(codepoint);
}

TGuiRectangle tgui_get_size_text_dim(tgui_s32 x, tgui_s32 y, char *text, tgui_u32 size) {
    TGuiRectangle result;
    
    tgui_s32 w = 0;
    tgui_s32 h = font.max_glyph_height;

    tgui_u32 text_len = size;
    for(tgui_u32 i = 0; i < text_len; ++i) {
        TGuiGlyph *glyph = font.glyphs + get_codepoint_index(text[i]);
        w += glyph->adv_width;
    }

    result.min_x = x;
    result.min_y = y;
    result.max_x = result.min_x + w - 1;
    result.max_y = result.min_y + h - 1;

    return result;
}

TGuiRectangle tgui_get_text_dim(tgui_s32 x, tgui_s32 y, char *text) {
    return tgui_get_size_text_dim(x, y, text, strlen(text));
}

void tgui_font_draw_text(TGuiPainter *painter, tgui_s32 x, tgui_s32 y, char *text, tgui_u32 size, tgui_u32 color) {
    tgui_s32 cursor = x;
    tgui_s32 base   = y + font.ascent;

    tgui_u32 text_len = size;
    
    tgui_u32 last_index = 0; TGUI_UNUSED(last_index);
    for(tgui_u32 i = 0; i < text_len; ++i) {
        
        tgui_u32 index = get_codepoint_index(text[i]);

        TGuiGlyph *glyph = font.glyphs + index;
        tgui_painter_draw_bitmap(painter, cursor + glyph->left_bearing, base - glyph->top_bearing, &glyph->bitmap, color);
        cursor += glyph->adv_width;

        last_index = index;
    }
}

/* --------------------------- */
/*       TGui Framebuffer      */
/* --------------------------- */

void tgui_texture(TGuiWindowHandle handle, void *texture) {
    TGuiWindow *window = tgui_window_get_from_handle(handle);
    if(!tgui_rect_invalid(window->dim)) {
        TGuiRenderBuffer *render_buffer = tgui_render_state_push_render_buffer_custom(&state.render_state, state.default_program, texture, NULL);
        TGuiPainter painter;
        tgui_painter_start(&painter, TGUI_PAINTER_TYPE_HARDWARE, window->dim, 0, NULL, render_buffer);
        tgui_painter_draw_render_buffer_texture(&painter, window->dim);
    }
}

/* ---------------------- */
/*       TGui Widgets     */
/* ---------------------- */

TGuiAllocatedWindow *tgui_allocated_window_node_alloc(void) {
    TGuiAllocatedWindow *result = NULL;
    if(state.free_windows) {
        result = state.free_windows;
        state.free_windows = state.free_windows->next;
    } else {
        result = tgui_arena_push_struct(&state.arena, TGuiAllocatedWindow, 8);
    }
    TGUI_ASSERT(result);
    memset(result, 0, sizeof(TGuiAllocatedWindow));

    return result;
}

void tgui_allocated_window_node_free(TGuiAllocatedWindow *allocated_window) {
    allocated_window->next = state.free_windows;
    state.free_windows = allocated_window;
}

TGuiWidget *tgui_widget_alloc(void) {
    TGuiWidget *widget = NULL;
    if(state.first_free_widget) {
        widget = state.first_free_widget;
        state.first_free_widget = state.first_free_widget->next;
    } else {
        widget = tgui_arena_push_struct(&state.arena, TGuiWidget, 8);
    }
    memset(widget, 0, sizeof(TGuiWidget));
    TGUI_ASSERT(widget);
    return widget;
}

void tgui_widget_free(TGuiWidget *widget)  {
    widget->next = state.first_free_widget;
    state.first_free_widget = widget;
}

void tgui_window_process_widgets(TGuiWindow *window, TGuiPainter *painter) {
    TGuiWidget *widget = window->widgets->next;
    TGuiRectangle saved_clip = painter->clip;
    while(!tgui_clink_list_end(widget, window->widgets)) {
        
        if(tgui_window_flag_is_set(window, TGUI_WINDOW_SCROLLING)) {
            widget->x -= window->h_scroll_offset * (tgui_rect_width(window->scroll_saved_rect) - tgui_rect_width(window->dim));
            widget->y -= window->v_scroll_offset * (tgui_rect_height(window->scroll_saved_rect) - tgui_rect_height(window->dim));
        }

        painter->clip = tgui_rect_intersection(window->dim, painter->clip);
        widget->internal(widget, painter);
        painter->clip = saved_clip;
        widget = widget->next;
    
    }
}

void tgui_window_free_widgets(TGuiWindow *window) {
    TGuiWidget *widget = window->widgets->next;
    while(!tgui_clink_list_end(widget, window->widgets)) {
        TGuiWidget *to_free = widget;
        widget = widget->next;
        tgui_clink_list_remove(to_free);
        tgui_widget_free(to_free);
    }
}

void *_tgui_widget_get_state(tgui_u64 id, tgui_u64 size) {
    void *result = NULL;

    result = tgui_virtual_map_find(&state.registry, id);
    if(result == NULL) {
        result = tgui_arena_alloc(&state.arena, size, 8);
        memset(result, 0, size);
        tgui_virtual_map_insert(&state.registry, id, (void *)result);
    }

    TGUI_ASSERT(result != NULL);
    return result;
}

tgui_b32 tgui_window_update_widget(TGuiWindow *window) {
    return tgui_docker_window_is_visible(window->parent, window);
}

static TGuiRectangle calculate_widget_rect(TGuiWidget *widget) {

    TGuiWindow *window = widget->parent;
    tgui_s32 x = widget->x;
    tgui_s32 y = widget->y;
    tgui_s32 w = widget->w;
    tgui_s32 h = widget->h;
    
    TGuiRectangle window_rect = window->dim;
    TGuiRectangle rect = {
        window_rect.min_x + x,
        window_rect.min_y + y,
        window_rect.min_x + x + w - 1,
        window_rect.min_y + y + h - 1
    };

    return rect;
}

tgui_u64 tgui_get_widget_id(char *tgui_id) {
    tgui_u64 id = tgui_hash(tgui_id, strlen(tgui_id));
    return id;
}

void tgui_calculate_hot_widget(TGuiWindow *window, TGuiRectangle rect, tgui_u64 id) {

    TGuiRectangle visible_rect = tgui_rect_intersection(rect, window->dim);
    tgui_b32 mouse_is_over = tgui_rect_point_overlaps(visible_rect, input.mouse_x, input.mouse_y);
    
    if(mouse_is_over && (!state.active || state.active == id)) {
        state.hot = id;
    }

    if(!mouse_is_over && state.hot == id) {
        state.hot = 0;
    }

    if(docker.grabbing_window == true) {
        state.hot = 0;
    }
}

void tgui_widget_alloc_into_window(tgui_u64 id, TGuiWidgetInternalFunc internal, TGuiWindow *window, tgui_u32 x, tgui_u32 y, tgui_u32 w, tgui_u32 h) {
    
    TGuiWidget *widget = tgui_widget_alloc();
    
    widget->id = id;
    widget->parent = window;
    widget->x = x;
    widget->y = y;
    widget->w = w;
    widget->h = h;
    widget->next = 0;
    widget->prev = 0;

    widget->internal = internal;

    tgui_clink_list_insert_back(window->widgets, widget);
}

void _tgui_label(TGuiWindowHandle handle, char *content, tgui_u32 color, tgui_s32 x, tgui_s32 y, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);

    if(!tgui_window_update_widget(window)) {
        return;
    }

    tgui_u64 id = tgui_get_widget_id(tgui_id);
    
    tgui_u32 content_size = TGUI_MIN(strlen(content), TGUI_MAX_LABEL_SIZE-1);

    TGuiLabel *label_state = tgui_widget_get_state(id, TGuiLabel);
    memcpy(label_state->content, content, content_size);
    label_state->size = content_size;
    label_state->content[content_size] = '\0';
    label_state->color = color;
    
    TGuiRectangle text_rect = tgui_get_size_text_dim(x, y, label_state->content, label_state->size);

    tgui_widget_alloc_into_window(id, _tgui_label_internal, window, x, y, tgui_rect_width(text_rect), tgui_rect_height(text_rect));
}

void _tgui_label_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiRectangle rect = calculate_widget_rect(widget); 
    tgui_calculate_hot_widget(window, rect, id);

    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(rect, window->dim);
    
    TGuiLabel *label_state = tgui_widget_get_state(id, TGuiLabel);
    tgui_font_draw_text(painter, rect.min_x, rect.min_y, label_state->content,  label_state->size, label_state->color);

    painter->clip = saved_painter_clip;
}


tgui_b32 _tgui_button(TGuiWindowHandle handle, char *label, tgui_s32 x, tgui_s32 y, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);

    if(!tgui_window_update_widget(window)) {
        return false;
    }

    tgui_u64 id = tgui_get_widget_id(tgui_id);

    TGuiButton *button_state = tgui_widget_get_state(id, TGuiButton);
    button_state->label = label;
    
    tgui_widget_alloc_into_window(id, _tgui_button_internal, window, x, y, 120, 30);

    return button_state->result;
}

void _tgui_button_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiRectangle rect = calculate_widget_rect(widget); 
    tgui_calculate_hot_widget(window, rect, id);

    tgui_b32 result = false;
    
    if(state.active == id) {
        if(!input.mouse_button_is_down && input.mouse_button_was_down) {
            if(state.hot == id) result = true;
            state.active = 0;
        }
    } else if(state.hot == id) {
        if(!input.mouse_button_was_down && input.mouse_button_is_down) {
            state.active = id;
        }    
    }

    /* TODO: Desing a command interface to render the complete UI independently */
    
    tgui_u32 button_color = 0x999999;
    tgui_u32 decoration_color = 0x222222;
    
    if(state.hot == id) {
        button_color = 0x888888;
    }

    if(state.active == id) {
        button_color = 0x777777;
    }
    
    /* TODO: All this rendering is temporal, the API should have a render independent way to give render primitives to the user */

    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(rect, window->dim);
    
    tgui_painter_draw_rectangle(painter, rect, button_color);
    
    TGuiButton *button_state = tgui_widget_get_state(id, TGuiButton);
    
    char *label = button_state->label;
    TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);
    
    tgui_s32 label_x = rect.min_x + (tgui_rect_width(rect) - 1) / 2 - (tgui_rect_width(label_rect) - 1) / 2;
    tgui_s32 label_y = rect.min_y + (tgui_rect_height(rect) - 1) / 2 - (tgui_rect_height(label_rect) - 1) / 2;
    tgui_font_draw_text(painter, label_x, label_y, label,  strlen(label), decoration_color);
    tgui_painter_draw_rectangle_outline(painter, rect, decoration_color);

    painter->clip = saved_painter_clip;
    
    button_state->result = result;
}

tgui_b32 _tgui_separator(TGuiWindowHandle handle, char *label, tgui_s32 y, tgui_b32 initial_value, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);
    
    tgui_u64 id = tgui_get_widget_id(tgui_id);
    
    if(!tgui_window_update_widget(window)) {
        if(state.active == id) {
            state.active = 0;
        }
        return false;
    }
    
    TGuiSeparator *separator = tgui_widget_get_state(id, TGuiSeparator);
    separator->label = label;

    if(!separator->initilize) {
        separator->open = initial_value;
        separator->initilize = true;
    }
    
    tgui_u32 h = font.max_glyph_height + 10;
    tgui_widget_alloc_into_window(id, _tgui_separator_internal, window, 0, y, 0, h);

    return separator->open;
}

void _tgui_separator_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiRectangle rect = calculate_widget_rect(widget);
    TGuiRectangle fake_separator_dim = rect;
    fake_separator_dim.min_x = window->dim.min_x;
    fake_separator_dim.max_x = window->dim.max_x;
    
    tgui_calculate_hot_widget(window, fake_separator_dim, id);

    TGuiSeparator *separator = tgui_widget_get_state(id, TGuiSeparator);

    if(state.active == id) {

    } else if(state.hot == id) {
        if(!input.mouse_button_was_down && input.mouse_button_is_down) {
            state.active = id;
        }
    }

    if(state.active == id) {
        separator->open = !separator->open;
        state.active = 0;
    }

    tgui_u32 color = 0x444444;
    if(separator->open == true) {
        color = 0x555555;
    }

    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(fake_separator_dim, painter->clip);
    
    tgui_painter_draw_rectangle(painter, fake_separator_dim, color);
    
    char *label = separator->label;
    TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);
    
    tgui_s32 label_x = rect.min_x + 24;
    tgui_s32 label_y = rect.min_y + (tgui_rect_height(rect) - 1) / 2 - (tgui_rect_height(label_rect) - 1) / 2;
    tgui_font_draw_text(painter, label_x, label_y, label,  strlen(label), 0xffffff);

    tgui_painter_draw_rectangle_outline(painter, fake_separator_dim, 0x888888);

    painter->clip = saved_painter_clip;

}

void _tgui_float_input(TGuiWindowHandle handle, tgui_f32 *value, tgui_u32 border_color, tgui_s32 x, tgui_s32 y, tgui_s32 w, char *tgui_id) {
    
    TGuiWindow *window = tgui_window_get_from_handle(handle);
    
    tgui_u64 id = tgui_get_widget_id(tgui_id);
    
    if(!tgui_window_update_widget(window)) {
        if(state.active == id) {
            state.active = 0;
        }
        return;
    }
    
    tgui_u32 h =  font.max_glyph_height + TGUI_FLOAT_INPUT_PADDING*2;

    TGuiFloatInput *float_input = tgui_widget_get_state(id, TGuiFloatInput);
    float_input->value = value;
    float_input->border_color = border_color;
    tgui_widget_alloc_into_window(id, _tgui_float_input_internal, window, x, y, w, h);
}

static inline void float_input_update_buffer(TGuiFloatInput *float_input) {
    snprintf(float_input->buffer, TGUI_FLOAT_INPUT_MAX_CHARACTERS, "%.2f", *float_input->value);
    float_input->used = strlen(float_input->buffer);
}

static inline void float_input_update_value(TGuiFloatInput *float_input) {
    *float_input->value = (tgui_f32)atof(float_input->buffer);
}

static inline tgui_b32 float_input_valid_character(tgui_u8 character) {
    if(tgui_is_digit(character)) {
        return true;
    }
    if(character == '.' || character == '-') {
        return true;
    }
    return false;
}


static inline void float_input_move_cursor_left(TGuiFloatInput *float_input) {
    if(float_input->cursor == 0) return;
    --float_input->cursor;
}

static inline void float_input_move_cursor_right(TGuiFloatInput *float_input) {
    if(float_input->cursor == float_input->used) return;
    ++float_input->cursor;
}

static inline void float_input_insert_at_cursor(TGuiFloatInput *float_input, tgui_u8 character) {
    if(float_input->used >= (TGUI_FLOAT_INPUT_MAX_CHARACTERS-1)) return;

    for(tgui_s32 index = float_input->used - 1; index >= (tgui_s32)float_input->cursor; --index) {
        float_input->buffer[index + 1] = float_input->buffer[index];
    }
    
    float_input->buffer[float_input->cursor] = character;

    ++float_input->used;
    ++float_input->cursor;

    float_input->buffer[float_input->used] = '\0';
}

static inline void float_input_remove_from_cursor(TGuiFloatInput *float_input) {
    if(float_input->cursor == 0 || float_input->used == 0) return;
    
    for(tgui_u32 index = float_input->cursor; index < float_input->used; ++index) {
        float_input->buffer[index - 1] = float_input->buffer[index];
    }

    --float_input->used;
    --float_input->cursor;

    float_input->buffer[float_input->used] = '\0';
}

void _tgui_float_input_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiKeyboard *keyboard = &input.keyboard;

    TGuiRectangle rect = calculate_widget_rect(widget);
    tgui_calculate_hot_widget(window, rect, id);
    
    TGuiFloatInput *float_input = tgui_widget_get_state(id, TGuiFloatInput);
    
    if(!float_input->initilize) {
        float_input_update_buffer(float_input);
        float_input->cursor = 0;
        float_input->saved_value = *float_input->value;

        float_input->initilize = true;
    }
    
    if(state.active == id) {
        
        if(input.text_size > 0) {
            for(tgui_u32 index = 0; index < input.text_size; ++index) {
                tgui_u8 character = input.text[index];
                if(float_input_valid_character(character)) {
                    float_input_insert_at_cursor(float_input, character);
                }
            }
        }

        if(keyboard->k_backspace) {
            float_input_remove_from_cursor(float_input);
        }

        if(keyboard->k_l_arrow_down) {
            float_input_move_cursor_left(float_input);
        }

        if(keyboard->k_r_arrow_down) {
            float_input_move_cursor_right(float_input);
        }

        float_input_update_value(float_input);

    } else if (state.hot == id) {
        if(!input.mouse_button_was_down && input.mouse_button_is_down) {
            state.active = id;
            float_input->cursor = 0;
        }
    
    } else {
        float_input_update_buffer(float_input);
    }
    

    if(state.active == id && state.hot != id && input.mouse_button_was_down && !input.mouse_button_is_down) {
        state.active = 0;
    }

    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(rect, window->dim);
    
    tgui_u32 color = 0x333333;
    if(state.active == id) {
        color = 0x222222;
    }

    tgui_painter_draw_rectangle(painter, rect, color);
    
    TGuiRectangle text_rect = tgui_get_size_text_dim(0, 0, float_input->buffer, float_input->used);
    
    tgui_u32 text_x = rect.min_x + tgui_rect_width(rect) / 2 - tgui_rect_width(text_rect) / 2;
    tgui_u32 text_y = rect.min_y + tgui_rect_height(rect) / 2 - tgui_rect_height(text_rect) / 2;

    tgui_font_draw_text(painter, text_x, text_y, float_input->buffer,  float_input->used, 0xffffff);
    
    if(state.active == id) {
        TGuiRectangle cursor_rect = {
            text_x + (float_input->cursor * font.max_glyph_width),
            text_y,
            text_x + (float_input->cursor * font.max_glyph_width),
            text_y + font.max_glyph_height
        };
        
        tgui_painter_draw_rectangle(painter, cursor_rect, 0xaaaaff);
    }

    tgui_painter_draw_rectangle_outline(painter, rect, float_input->border_color);

    painter->clip = saved_painter_clip;
}

static TGuiRectangle calculate_selection_rect(TGuiTextInput *text_input, tgui_s32 x, tgui_s32 y, tgui_u32 start, tgui_u32 end) {

    if(start > end) {
        tgui_u32 temp = start;
        start = end;
        end = temp;
    }
    TGUI_ASSERT(start <= end);

    TGuiRectangle result = {
        x + (start - text_input->offset) * font.max_glyph_width,
        y,
        x + (end - text_input->offset) * font.max_glyph_width,
        y + font.max_glyph_height,
    };

    return result;
}

static void delete_selection(TGuiTextInput *text_input) {
    
    tgui_u32 start = text_input->selection_start;
    tgui_u32 end = text_input->selection_end;

    if(start > end) {
        tgui_u32 temp = start;
        start = end;
        end = temp;
    }
    TGUI_ASSERT(start <= end);

    TGUI_ASSERT(text_input->selection == true);
    memmove(text_input->buffer + start,
            text_input->buffer + end,
            text_input->used - end);
    
    TGUI_ASSERT((end - start) <= text_input->used);
    
    text_input->used -= (end - start);
    text_input->selection = false;
    text_input->cursor = start;
    if(text_input->offset > start) {
        text_input->offset = start;
    }
}

TGuiTextInput *_tgui_text_input(TGuiWindowHandle handle, tgui_s32 x, tgui_s32 y, char *tgui_id) {
    
    TGuiWindow *window = tgui_window_get_from_handle(handle);

    tgui_u64 id = tgui_get_widget_id(tgui_id);

    TGuiTextInput *text_input = tgui_widget_get_state(id, TGuiTextInput);

    if(!tgui_window_update_widget(window)) {
        if(state.active == id) {
            state.active = 0;
        }
        return text_input;
    }
    
    tgui_widget_alloc_into_window(id, _tgui_text_input_internal, window, x, y, 140, 30);

    return text_input; 
}

void _tgui_text_input_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiKeyboard *keyboard = &input.keyboard;

    TGuiRectangle rect = calculate_widget_rect(widget);
    tgui_calculate_hot_widget(window, rect, id);
    
    TGuiTextInput *text_input = tgui_widget_get_state(id, TGuiTextInput);
    
    if(!text_input->initilize) {
        
        text_input->selection = false;
        text_input->selection_start = 0;
        text_input->selection_end = 0;
        
        text_input->cursor_inactive_acumulator = 0;
        text_input->cursor_inactive_target = 0.5f;
        text_input->cursor_blink_target = 0.4f; 
        text_input->cursor_blink_acumulator = 0; 
        text_input->blink_cursor = false;
        text_input->draw_cursor  = true;

        text_input->initilize = true;
    }
    
    text_input->cursor_inactive_acumulator += state.dt;
    if(text_input->cursor_inactive_acumulator >= text_input->cursor_inactive_target) {
        text_input->blink_cursor = true;
    }

    if(text_input->blink_cursor) {
        text_input->cursor_blink_acumulator += state.dt;
        if(text_input->cursor_blink_acumulator >= text_input->cursor_blink_target) {
            text_input->draw_cursor = !text_input->draw_cursor;
            text_input->cursor_blink_acumulator = 0;
        }
    }

    TGuiRectangle visible_rect = tgui_rect_intersection(rect, window->dim);
    tgui_u32 padding_x = 8;
    tgui_u32 visible_glyphs = TGUI_MAX((tgui_s32)((tgui_rect_width(visible_rect) - padding_x*2)/font.max_glyph_width), (tgui_s32)0);

    if(state.active == id) {
        
        if(keyboard->k_r_arrow_down) {
            
            /* NOTE: Not blick cursor */
            text_input->blink_cursor = false;
            text_input->draw_cursor = true;
            text_input->cursor_inactive_acumulator = 0;

            /* NOTE: Start selection */
            if(!keyboard->k_shift) {
                text_input->selection = false;
            } else if(keyboard->k_shift && !text_input->selection) {
                text_input->selection = true;
                text_input->selection_start = text_input->cursor;
            }

            text_input->cursor = TGUI_MIN(text_input->cursor + 1, text_input->used);

            /* NOTE: End selection */
            if(text_input->selection) {
                text_input->selection_end = text_input->cursor;
                TGUI_ASSERT(TGUI_ABS((tgui_s32)text_input->selection_end - (tgui_s32)text_input->selection_start) <= (tgui_s32)text_input->used);
            }
  
        } else if(keyboard->k_l_arrow_down) {

            /* NOTE: Not blick cursor */
            text_input->blink_cursor = false;
            text_input->draw_cursor = true;
            text_input->cursor_inactive_acumulator = 0;
        
            /* NOTE: Start selection */
            if(!keyboard->k_shift) {
                text_input->selection = false;
            } else if(keyboard->k_shift && !text_input->selection) {
                text_input->selection = true;
                text_input->selection_start = text_input->cursor;
            }
            
            text_input->cursor = TGUI_MAX((tgui_s32)text_input->cursor - 1, 0);

            /* NOTE: End selection */
            if(text_input->selection) {
                text_input->selection_end = text_input->cursor;
                TGUI_ASSERT(TGUI_ABS((tgui_s32)text_input->selection_end - (tgui_s32)text_input->selection_start) <= (tgui_s32)text_input->used);
            }
        
        } else if(keyboard->k_backspace) {

            /* NOTE: Not blick cursor */
            text_input->blink_cursor = false;
            text_input->draw_cursor = true;
            text_input->cursor_inactive_acumulator = 0;
            
            if(text_input->selection) {
                delete_selection(text_input);
            } else if(text_input->cursor > 0) {
                memmove(text_input->buffer + text_input->cursor - 1,
                        text_input->buffer + text_input->cursor,
                        text_input->used - text_input->cursor);
                text_input->cursor -= 1;
                text_input->used   -= 1;
            }
        
        } else if(keyboard->k_delete) {

            /* NOTE: Not blick cursor */
            text_input->blink_cursor = false;
            text_input->draw_cursor = true;
            text_input->cursor_inactive_acumulator = 0;

            if(text_input->selection) {
                delete_selection(text_input);
            } else if(text_input->cursor < text_input->used) {
                memmove(text_input->buffer + text_input->cursor,
                        text_input->buffer + text_input->cursor + 1,
                        text_input->used - text_input->cursor - 1);
                text_input->used -= 1;
            }

        } else if(input.text_size > 0) {

            /* NOTE: Not blick cursor */
            text_input->blink_cursor = false;
            text_input->draw_cursor = true;
            text_input->cursor_inactive_acumulator = 0;

            if(text_input->selection) {
                delete_selection(text_input);
            }

            if((text_input->used + input.text_size) > TGUI_TEXT_INPUT_MAX_CHARACTERS) {
                input.text_size = (TGUI_TEXT_INPUT_MAX_CHARACTERS - text_input->used);
                TGUI_ASSERT((text_input->used + input.text_size) == TGUI_TEXT_INPUT_MAX_CHARACTERS);
            }

            memmove(text_input->buffer + text_input->cursor + input.text_size, 
                    text_input->buffer + text_input->cursor, 
                    (text_input->used - text_input->cursor));
            memcpy(text_input->buffer + text_input->cursor, input.text, input.text_size);

            text_input->used   += input.text_size;
            text_input->cursor += input.text_size;
        }
        
        /* Calculate the ofset of the text */
        if(text_input->cursor > text_input->offset + visible_glyphs) {
            text_input->offset += 1;
        } else if(text_input->offset > 0 && text_input->cursor < text_input->offset) {
            text_input->offset -= 1;
        }
    
    } else if(state.hot == id) {
        if(!input.mouse_button_was_down && input.mouse_button_is_down) {
            state.active = id;
            /* TODO: Find a better aproach to handle text input */
            input.text_size = 0;
        }    
    }

    tgui_b32 mouse_is_over = tgui_rect_point_overlaps(visible_rect, input.mouse_x, input.mouse_y);
    if(state.active == id && !mouse_is_over && input.mouse_button_was_down && !input.mouse_button_is_down) {
        state.active = 0;
    }
 
    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = window->dim; 
    
    tgui_u32 color = 0x888888;
    tgui_u32 decoration_color = 0x333333;
    tgui_u32 cursor_color = 0x00ff00;
    
    if(state.hot == id) {
        color = 0x888888;
    }
    
    if(state.active == id) {
        color = 0x999999;
        decoration_color = 0x333333;
    }

    tgui_painter_draw_rectangle(painter, rect, color);
    tgui_painter_draw_rectangle_outline(painter, rect, decoration_color);

    TGuiRectangle clipping_rect = rect;
    clipping_rect.min_x += padding_x;
    clipping_rect.max_x -= padding_x;
    painter->clip = tgui_rect_intersection(clipping_rect, painter->clip);
    
    tgui_s32 text_x = rect.min_x + padding_x;
    tgui_s32 text_y = rect.min_y + ((tgui_rect_height(rect) - 1) / 2) - ((font.max_glyph_height - 1) / 2);
    

    if(text_input->selection) {
        TGuiRectangle selection_rect = calculate_selection_rect(text_input, text_x, text_y, text_input->selection_start, text_input->selection_end);
        tgui_painter_draw_rectangle(painter, selection_rect, 0x7777ff);
    }

    tgui_font_draw_text(painter, text_x, text_y, (char *)text_input->buffer + text_input->offset,
            text_input->used - text_input->offset, decoration_color);

    if(state.active == id && text_input->draw_cursor) {
        TGuiRectangle cursor_rect = {
            text_x + ((text_input->cursor - text_input->offset) * font.max_glyph_width),
            text_y,
            text_x + ((text_input->cursor - text_input->offset) * font.max_glyph_width),
            text_y + font.max_glyph_height,
        };
        tgui_painter_draw_rectangle(painter, cursor_rect, cursor_color);
    }
    
    painter->clip = saved_painter_clip;

}

static tgui_u32 tgui_u32_color_mix(tgui_u32 color0, tgui_f32 t, tgui_u32 color1) {

    tgui_u32 r0 = ((color0 >> 16) & 0xff);
    tgui_u32 g0 = ((color0 >>  8) & 0xff);
    tgui_u32 b0 = ((color0 >>  0) & 0xff);

    tgui_u32 r1 = ((color1 >> 16) & 0xff);
    tgui_u32 g1 = ((color1 >>  8) & 0xff);
    tgui_u32 b1 = ((color1 >>  0) & 0xff);

    tgui_u32 cr = (r0 * (1.0f - t) + r1 * t);
    tgui_u32 cg = (g0 * (1.0f - t) + g1 * t);
    tgui_u32 cb = (b0 * (1.0f - t) + b1 * t);
    
    return (0xff << 24) | (cr << 16) | (cg << 8) | (cb << 0);
}

void colorpicker_calculate_radiant(TGuiBitmap *bitmap, tgui_u32 color) {
    
    tgui_u32 w = (bitmap->width - 1) != 0 ? (bitmap->width - 1) : 1;
    tgui_f32 inv_w = 1.0f / w;

    tgui_u32 *pixel = bitmap->pixels;
    for(tgui_u32 x = 0; x < bitmap->width; ++x) { 
        tgui_f32 t = (tgui_f32)x * inv_w;
        *pixel++ = tgui_u32_color_mix(0xffffff, t, color);
    }
    
    tgui_u32 h = (bitmap->height - 1) != 0 ? (bitmap->height - 1) : 1;
    tgui_f32 inv_h = 1.0f / h;
    
    for(tgui_u32 x = 0; x < bitmap->width; ++x) {
        tgui_u32 color = bitmap->pixels[x];
        for(tgui_u32 y = 1; y < bitmap->height; ++y) {
            tgui_f32 t = (tgui_f32)y * inv_h;
            bitmap->pixels[y*bitmap->width+x] = tgui_u32_color_mix(color, t, 0x000000);
        }
    } 


}

static void colorpicker_claculate_section(TGuiBitmap *bitmap, tgui_u32 *cursor_x, tgui_u32 advance, tgui_u32 color0, tgui_u32 color1) {

    tgui_f32 inv_advance = advance != 0 ? (1.0f / advance) : 1.0f;

    for(tgui_u32 y = 0; y < bitmap->height; ++y) {
        tgui_u32 xx = *cursor_x;
        for(tgui_u32 x = 0; x < advance; ++x) {
            tgui_f32 t = x * inv_advance;
            bitmap->pixels[y*bitmap->width+xx++] = tgui_u32_color_mix(color0, t, color1);
        }
    }

    *cursor_x += advance;
}

void colorpicker_calculate_mini_radiant(TGuiBitmap *bitmap) {
    
    tgui_u32 advance = 0;
    tgui_u32 cursor_x = 0;
    
    tgui_u32 color0 = 0xff0000;
    tgui_u32 color1 = 0xffff00;
    advance = (bitmap->width * 1.0f/6.0f);
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);
    
    color0 = 0xffffff00;
    color1 = 0xff00ff00;
    advance = (bitmap->width * 2.0f/6.0f);
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);
    
    color0 = 0x00ff00;
    color1 = 0x00ffff;
    advance = (bitmap->width * 3.0f/6.0f);
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);
    
    color0 = 0x00ffff;
    color1 = 0x0000ff;
    advance = (bitmap->width * 4.0f/6.0f);
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);
    
    color0 = 0x0000ff;
    color1 = 0xff00ff;
    advance = (bitmap->width * 5.0f/6.0f);
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);
    
    color0 = 0xff00ff;
    color1 = 0xff0000;
    advance = bitmap->width;
    colorpicker_claculate_section(bitmap, &cursor_x, advance - cursor_x, color0, color1);

}

void tgui_tgui_u32_color_to_hsv_color(tgui_u32 color, tgui_f32 *h, tgui_f32 *s, tgui_f32 *v) {
    
    tgui_s32 r = ((color >> 16) & 0xff);// / 255.0f;
    tgui_s32 g = ((color >>  8) & 0xff);// / 255.0f;
    tgui_s32 b = ((color >>  0) & 0xff);// / 255.0f;

    tgui_s32 M = TGUI_MAX3(r, g, b);
    tgui_s32 m = TGUI_MIN3(r, g, b);
    tgui_f32 chroma = M - m;
    
    tgui_f32 hue = 0;
    if(chroma > 0.00001f) {
        if(M <= r) {
            hue = fmod((g-b)/chroma, 6);
        } else if(M <= g) {
            hue = 2.0f + (b-r)/chroma;
        } else if(M <= b) {
            hue = 4.0f + (r-g)/chroma;
        }
    }
    hue *= 60;
    if(hue < 0) hue += 360;

    tgui_f32 saturation = 0;
    if(M != 0) {
        saturation = chroma / (tgui_f32)M;
    }
    tgui_f32 value = M / 255.0f;

    *h = hue / 360.0f;
    *s = saturation;
    *v = value;
}

static tgui_u32 colorpicker_get_color_hue(TGuiColorPicker *colorpicker) {
    return colorpicker->mini_radiant.pixels[(tgui_u32)(colorpicker->hue * (colorpicker->mini_radiant.width - 0))];
}

void _tgui_color_picker(TGuiWindowHandle handle, tgui_s32 x, tgui_s32 y, tgui_s32 w, tgui_s32 h, tgui_u32 *color, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);

    if(!tgui_window_update_widget(window)) {
        return;
    }
    tgui_u64 id = tgui_get_widget_id(tgui_id);
    
    TGuiColorPicker *colorpicker = tgui_widget_get_state(id, TGuiColorPicker);
    colorpicker->color_ptr = color;
    
    tgui_widget_alloc_into_window(id, _tgui_color_picker_internal, window, x, y, w, h);

}

void _tgui_color_picker_internal(TGuiWidget *widget, TGuiPainter *painter) {
    
    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    tgui_u32 w = widget->w;
    tgui_u32 h = widget->h;
    
    TGuiRectangle rect = calculate_widget_rect(widget);
    tgui_calculate_hot_widget(window, rect, id);

    TGuiColorPicker *colorpicker = tgui_widget_get_state(id, TGuiColorPicker);

    /* TODO: Actually use hue saturation and value from the color */
    if(colorpicker->color_ptr) {
        tgui_f32 hue, saturation, value;
        tgui_tgui_u32_color_to_hsv_color(*colorpicker->color_ptr, &hue, &saturation, &value);
    }

    tgui_f32 radiant_h = h * 0.75f; 
    tgui_f32 mini_radiant_h = h * 0.2f;
    
    if(!colorpicker->initialize) {
         
        colorpicker->mini_radiant = tgui_bitmap_alloc_empty(&state.arena, w, mini_radiant_h);
        colorpicker_calculate_mini_radiant(&colorpicker->mini_radiant);
        
        colorpicker->saved_radiant_color = colorpicker_get_color_hue(colorpicker);
        colorpicker->radiant = tgui_bitmap_alloc_empty(&state.arena, w, radiant_h);
        colorpicker_calculate_radiant(&colorpicker->radiant, colorpicker->saved_radiant_color);
        
        colorpicker->sv_cursor_active = false;
        colorpicker->hue_cursor_active = false;

        colorpicker->initialize = true;
    }

    TGuiRectangle radiant_rect = tgui_rect_from_wh(rect.min_x, rect.min_y, colorpicker->radiant.width, colorpicker->radiant.height);
    TGuiRectangle mini_radiant_rect = tgui_rect_from_wh(rect.min_x, rect.max_y - mini_radiant_h, colorpicker->mini_radiant.width, colorpicker->mini_radiant.height);

    if(state.hot == id) {
        tgui_b32 mouse_is_over = tgui_rect_point_overlaps(mini_radiant_rect, input.mouse_x, input.mouse_y); 
        if(mouse_is_over && input.mouse_button_is_down && !input.mouse_button_was_down) {
            state.active = id;
            colorpicker->hue_cursor_active = true;
        }

        mouse_is_over = tgui_rect_point_overlaps(radiant_rect, input.mouse_x, input.mouse_y); 
        if(mouse_is_over && input.mouse_button_is_down && !input.mouse_button_was_down) {
            state.active = id;
            colorpicker->sv_cursor_active = true;
        }
    }
    
    if(state.active == id && input.mouse_button_was_down && !input.mouse_button_is_down) {
        state.active = 0;
        colorpicker->sv_cursor_active = false;
        colorpicker->hue_cursor_active = false;
    }

    if(colorpicker->hue_cursor_active) {
        colorpicker->hue = TGUI_CLAMP((input.mouse_x - mini_radiant_rect.min_x) / (tgui_f32)colorpicker->mini_radiant.width, 0, 1);
    }

    if(colorpicker->sv_cursor_active) {
        colorpicker->saturation =  TGUI_CLAMP((input.mouse_x - radiant_rect.min_x) / (tgui_f32)colorpicker->radiant.width, 0, 1);
        colorpicker->value      =  TGUI_CLAMP((input.mouse_y - radiant_rect.min_y) / (tgui_f32)colorpicker->radiant.height, 0, 1);
    }

    tgui_u32 cursor_w = 6;
    TGuiRectangle hue_cursor = mini_radiant_rect;

    hue_cursor.min_x += (colorpicker->hue * (colorpicker->mini_radiant.width - cursor_w)); 
    hue_cursor.max_x = hue_cursor.min_x + cursor_w - 1;  

    tgui_u32 radiant_color = colorpicker_get_color_hue(colorpicker);
    if(colorpicker->saved_radiant_color != radiant_color) {
        colorpicker_calculate_radiant(&colorpicker->radiant, radiant_color);
        colorpicker->saved_radiant_color = radiant_color;
    }
    
    TGuiRectangle saved_clip = painter->clip;
    painter->clip = tgui_rect_intersection(painter->clip, window->dim);

    tgui_u32 color_x = colorpicker->saturation * (colorpicker->radiant.width -  1);
    tgui_u32 color_y = colorpicker->value * (colorpicker->radiant.height - 1);
   
#if 0 
    painter_draw_bitmap_no_alpha(painter, radiant_rect.min_x, radiant_rect.min_y, &colorpicker->radiant);
#endif
    tgui_painter_draw_rectangle_outline(painter, radiant_rect, 0x444444);

    tgui_painter_draw_hline(painter, radiant_rect.min_y + color_y, radiant_rect.min_x, radiant_rect.max_x, 0x444444);
    tgui_painter_draw_vline(painter, radiant_rect.min_x + color_x, radiant_rect.min_y, radiant_rect.max_y, 0x444444);
    
#if 0 
    painter_draw_bitmap_no_alpha(painter, mini_radiant_rect.min_x, mini_radiant_rect.min_y, &colorpicker->mini_radiant);
#endif
    tgui_painter_draw_rectangle_outline(painter, mini_radiant_rect, 0x444444);
    
    painter->clip = tgui_rect_intersection(window->dim, mini_radiant_rect);

    tgui_painter_draw_rectangle(painter, hue_cursor, 0x444444);
    tgui_painter_draw_rectangle_outline(painter, hue_cursor, 0x888888);

    painter->clip = saved_clip;
    
    if(colorpicker->color_ptr) {
        *colorpicker->color_ptr = colorpicker->radiant.pixels[color_y*colorpicker->radiant.width + color_x];
    }
}

TGuiTreeViewNode *treeview_alloc_node(TGuiTreeView *treeview, void *user_data) {
    TGuiTreeViewNode *node = NULL;
    if(treeview->first_free_node) {
        node = treeview->first_free_node;
        treeview->first_free_node = treeview->first_free_node->next;
    } else {
        node = tgui_arena_push_struct(&state.arena, TGuiTreeViewNode, 8);
    }
    TGUI_ASSERT(node);
    memset(node, 0, sizeof(TGuiTreeViewNode));
    node->user_data = user_data;
    return node;
}

void treeview_free_node(TGuiTreeView *treeview, TGuiTreeViewNode *node) {
    if(node->childs) {
        
        TGuiTreeViewNode *child = node->childs->next;
        while(!tgui_clink_list_end(child, node->childs)) {
            
            TGuiTreeViewNode *to_free = child;
            child = child->next;

            treeview_free_node(treeview, to_free);

        }

        treeview_free_node(treeview, node->childs);

    } 
   
    node->next = treeview->first_free_node;
    treeview->first_free_node = node; 
}

TGuiTreeViewNode *treeview_alloc_root_node(TGuiTreeView *treeview, void *user_data) {
    TGuiTreeViewNode *node = treeview_alloc_node(treeview, user_data);
    node->childs = treeview_alloc_node(treeview, NULL);
    tgui_clink_list_init(node->childs);
    return node;
}

void treeview_insert_node(TGuiTreeViewNode *node, TGuiTreeViewNode *parent) {
    tgui_clink_list_insert_back(parent->childs, node);
    node->parent = parent;
}


void _tgui_tree_view_begin(TGuiWindowHandle handle, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);

    tgui_u32 id = tgui_get_widget_id(tgui_id);

    state.active_id = id;
    state.active_window = window;
    
    if(!tgui_window_update_widget(window)) {
        return;
    }
    
    TGuiTreeView *treeview = tgui_widget_get_state(state.active_id, TGuiTreeView);
    treeview->dim = (TGuiRectangle){ 0, 0, 0, 0 };

    if(!treeview->initiliaze) {

        treeview->first_free_node = 0;
        
        treeview->rect_w = 8;
        treeview->padding = 6;

        treeview->selection_index = -1;
        treeview->selection_data = NULL;

        tgui_array_initialize(&treeview->root_node_state);
        tgui_array_initialize(&treeview->selected_node_data);

        tgui_array_reserve(&treeview->root_node_state,    256);
        tgui_array_reserve(&treeview->selected_node_data, 256);
        for(tgui_u32 i = 0; i < tgui_array_size(&treeview->root_node_state); ++i) {
            tgui_b32 *root_state = tgui_array_get_ptr(&treeview->root_node_state, i);
            *root_state = true;
        }

        treeview->initiliaze = true;
    }

    if(treeview->root) {
        treeview_free_node(treeview, treeview->root);
    }

    treeview->root = treeview_alloc_root_node(treeview, NULL);
    treeview->root->label = "dummy root node";

    treeview->active_root_node = treeview->root;
    treeview->root_node_state_head = 0;
    treeview->selected_node_data_head = 0;
    treeview->active_depth = 0;
}

void _tgui_tree_view_deselect(char *tgui_id) {
    tgui_u32 id = tgui_get_widget_id(tgui_id);

    TGuiTreeView *treeview = tgui_widget_get_state(id, TGuiTreeView);
    if(treeview) {
        treeview->selection_index = 0;
        treeview->selection_data = NULL;
    } 
}

void _tgui_tree_view_end(void **selected_data) {

    if(!tgui_window_update_widget(state.active_window)) {
        return;
    }

    TGuiTreeView *treeview = tgui_widget_get_state(state.active_id, TGuiTreeView);
    tgui_s32 x = treeview->dim.min_x;
    tgui_s32 y = treeview->dim.min_y;
    tgui_s32 w = tgui_rect_width(treeview->dim);
    tgui_s32 h = tgui_rect_height(treeview->dim);

    tgui_widget_alloc_into_window(state.active_id, _tgui_tree_view_internal, state.active_window, x, y, w, h);

    state.active_window = NULL;
    state.active_id = -1;

    if(treeview->selection_index >= 0) {
        *selected_data = treeview->selection_data;
    } else {
        *selected_data = NULL;
    }
}

void treeview_node_setup(TGuiTreeView *treeview, TGuiTreeViewNode *node, char *label, tgui_s32 depth) {
    TGUI_UNUSED(treeview);

    node->label = label;

    node->selected_state_index = treeview->selected_node_data_head++;
    if(node->selected_state_index >= tgui_array_size(&treeview->selected_node_data)) {
        tgui_array_push(&treeview->selected_node_data);
    }
    TGUI_ASSERT(treeview->selected_node_data_head <= tgui_array_size(&treeview->selected_node_data));

    if(node->childs) {
        node->state_index = treeview->root_node_state_head++;
        if(node->state_index >= tgui_array_size(&treeview->root_node_state)) {
            tgui_array_push(&treeview->root_node_state);
        }
        TGUI_ASSERT(treeview->root_node_state_head <= tgui_array_size(&treeview->root_node_state));
    }

    node->label_depth = depth;
    
    if((node->parent && node->parent == treeview->root) || (node->parent && tgui_array_get(&treeview->root_node_state, node->parent->state_index) && node->parent->visible)){
        
        tgui_u32 depth_in_pixels = depth*TGUI_TREEVIEW_DEFAULT_DEPTH_WIDTH;
        tgui_u32 x = treeview->padding*2 + treeview->rect_w + depth_in_pixels;
        
        TGuiRectangle text_label = tgui_get_text_dim(x, treeview->dim.max_y+1, label);
        treeview->dim = tgui_rect_union(treeview->dim, text_label);

        node->dim = text_label;
        node->dim.min_x = depth_in_pixels;
        node->dim.max_x = text_label.max_x;
        
        node->visible = true;
    } else {
        node->visible = false;
    }
}

void _tgui_tree_view_root_node_begin(char *label, void *user_data) {
    if(!tgui_window_update_widget(state.active_window)) {
        return;
    }

    
    TGuiTreeView *treeview = tgui_widget_get_state(state.active_id, TGuiTreeView);
   
    TGuiTreeViewNode *node = treeview_alloc_root_node(treeview, user_data);
    treeview_insert_node(node, treeview->active_root_node);
    treeview_node_setup(treeview, node, label, treeview->active_depth);
    
    treeview->active_root_node = node;
    treeview->active_depth += 1;
}

void _tgui_tree_view_root_node_end(void) {
    if(!tgui_window_update_widget(state.active_window)) {
        return;
    }

    TGuiTreeView *treeview = tgui_widget_get_state(state.active_id, TGuiTreeView);
    treeview->active_root_node = treeview->active_root_node->parent;
    treeview->active_depth -= 1;
}

void _tgui_tree_view_node(char *label, void *user_data) {

    if(!tgui_window_update_widget(state.active_window)) {
        return;
    }
    
    TGuiTreeView *treeview = tgui_widget_get_state(state.active_id, TGuiTreeView);

    TGuiTreeViewNode *node = treeview_alloc_node(treeview, user_data);
    treeview_insert_node(node, treeview->active_root_node);
    treeview_node_setup(treeview, node, label, treeview->active_depth);

}

void tree_view_translate_node_dim(TGuiTreeView *treeview, TGuiTreeViewNode *node) {

    tgui_s32 x = node->dim.min_x;
    tgui_s32 y = node->dim.min_y;
    tgui_s32 w = tgui_rect_width(node->dim);
    tgui_s32 h = tgui_rect_height(node->dim);
    
    TGuiRectangle treeview_rect = treeview->dim; 
    TGuiRectangle result = {
        treeview_rect.min_x + x,
        treeview_rect.min_y + y,
        treeview_rect.min_x + x + w - 1,
        treeview_rect.min_y + y + h - 1
    };

    node->dim = result;
}

TGuiRectangle treeview_get_fake_dim(TGuiTreeView *treeview, TGuiWindow *window) {
    TGuiRectangle result = treeview->dim;
    result.min_x = window->dim.min_x;
    result.max_x = window->dim.max_x;
    return result;
}

TGuiRectangle treeview_node_get_fake_dim(TGuiTreeViewNode *node, TGuiWindow *window) {
    TGuiRectangle result = node->dim;
    result.min_x = window->dim.min_x;
    result.max_x = window->dim.max_x;
    return result;
}

TGuiRectangle treeview_calculate_node_cruz_rect(TGuiTreeView *treeview, TGuiTreeViewNode *node) {
    tgui_s32 x = node->dim.min_x + treeview->rect_w/2;
    tgui_s32 y = node->dim.min_y + treeview->rect_w/2;
    TGuiRectangle result = tgui_rect_from_wh(x, y, treeview->rect_w, treeview->rect_w);
    return result;
}

void treeview_node_draw(TGuiWidget *widget, TGuiTreeView *treeview, TGuiTreeViewNode *node, TGuiPainter *painter, tgui_u32 *color) {
    if(!node->visible) return;

    if(*color == TGUI_TREEVIEW_COLOR0) {
        *color = TGUI_TREEVIEW_COLOR1;
    } else {
        *color = TGUI_TREEVIEW_COLOR0;
    }

    TGuiWindow *window = widget->parent;

    TGuiRectangle fake_node_dim = treeview_node_get_fake_dim(node, window);
    
    tgui_u32 rect_w = treeview->rect_w;
    tgui_u32 padding = treeview->padding;

    tgui_b32 mouse_in_node = tgui_rect_point_overlaps(fake_node_dim, input.mouse_x, input.mouse_y);
    tgui_u32 _color = *color;
    if(state.hot == widget->id && mouse_in_node) {
        _color = 0x777777;
    }
    
    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(fake_node_dim, painter->clip);

    tgui_painter_draw_rectangle(painter, fake_node_dim, _color);

    if((tgui_u32)treeview->selection_index == node->selected_state_index && tgui_array_get(&treeview->selected_node_data, treeview->selection_index)) {
        _color = 0xaaaaff;
        tgui_painter_draw_rectangle(painter, fake_node_dim, _color);
    }


    tgui_s32 label_x = node->dim.min_x + treeview->rect_w/2;
    tgui_font_draw_text(painter, label_x+rect_w+padding, node->dim.min_y, node->label, strlen(node->label), 0x333333);

    for(tgui_u32 i = 0; i < node->label_depth; ++i) {
        tgui_u32 depth_in_pixel = TGUI_TREEVIEW_DEFAULT_DEPTH_WIDTH * ((node->label_depth-1) - i);
        tgui_painter_draw_vline(painter, node->dim.min_x - treeview->rect_w - depth_in_pixel, node->dim.min_y, node->dim.max_y, 0x333333);
    }
    
    if(node->childs) {
        
        TGuiRectangle cruz_rect = treeview_calculate_node_cruz_rect(treeview, node);

        if(tgui_array_get(&treeview->root_node_state, node->state_index) == false) {
            TGuiRectangle cruz_rect_v = cruz_rect;
            cruz_rect_v.min_x += treeview->rect_w/4;
            cruz_rect_v.max_x -= treeview->rect_w/4;
            tgui_painter_draw_rectangle(painter, cruz_rect_v, 0x333333);

            TGuiRectangle cruz_rect_h = cruz_rect;
            cruz_rect_h.min_y += treeview->rect_w/4;
            cruz_rect_h.max_y -= treeview->rect_w/4;
            tgui_painter_draw_rectangle(painter, cruz_rect_h, 0x333333);
        } else {
            TGuiRectangle cruz_rect_h = cruz_rect;
            cruz_rect_h.min_y += treeview->rect_w/4;
            cruz_rect_h.max_y -= treeview->rect_w/4;
            tgui_painter_draw_rectangle(painter, cruz_rect_h, 0x333333);
        }

    } 
    
    painter->clip = saved_painter_clip;
    
    if(node->childs) {

        TGuiTreeViewNode *child = node->childs->next;
        while(!tgui_clink_list_end(child, node->childs)) {
            treeview_node_draw(widget, treeview, child, painter, color);
            child = child->next;
        }
    }
    
}

void treeview_translate_node(TGuiTreeView *treeview, TGuiTreeViewNode *node) {
    if(!node->visible) return;
    
    tree_view_translate_node_dim(treeview, node);

    if(node->childs) {
        TGuiTreeViewNode *child = node->childs->next;
        while(!tgui_clink_list_end(child, node->childs)) {
            treeview_translate_node(treeview, child);
            child = child->next;
        }
    }
}

void treeview_update_node(TGuiWidget *widget, TGuiTreeView *treeview, TGuiTreeViewNode *node) {
    if(!node->visible) return;
    
    TGuiWindow *window = widget->parent;

    TGuiRectangle fake_node_dim = treeview_node_get_fake_dim(node, window);

    tgui_u64 id = widget->id;
    tgui_b32 mouse_in_node = tgui_rect_point_overlaps(fake_node_dim, input.mouse_x, input.mouse_y);

    if(state.hot == id) {
        if(mouse_in_node && input.mouse_button_is_down && !input.mouse_button_was_down) {
            state.active = id;
        }
    }
    
    TGuiRectangle cruz_rect = treeview_calculate_node_cruz_rect(treeview, node);
    tgui_b32 on_cruz = tgui_rect_point_overlaps(cruz_rect, input.mouse_x, input.mouse_y);
    
    if(state.active == id) {
        
        if(on_cruz && node->childs) {
            
            if(!input.mouse_button_is_down) {
                tgui_b32 *root_node_state = tgui_array_get_ptr(&treeview->root_node_state, node->state_index);
                *root_node_state = !(*root_node_state);
                state.active = 0;
            } 

        } else {
            if(mouse_in_node && input.mouse_button_is_down) {
                
                if(treeview->selection_index >= 0) {
                    tgui_b32 *selected_node_data = tgui_array_get_ptr(&treeview->selected_node_data, treeview->selection_index); 
                    *selected_node_data = false;
                }

                treeview->selection_data = node->user_data;
                treeview->selection_index = node->selected_state_index;

                tgui_b32 *selected_node_data = tgui_array_get_ptr(&treeview->selected_node_data, treeview->selection_index); 
                *selected_node_data = true;
                state.active = 0;
            }
        }


        if(state.hot != id && !input.mouse_button_is_down) {
            state.active = 0;
        }


    }

    if(node->childs) {
        TGuiTreeViewNode *child = node->childs->next;
        while(!tgui_clink_list_end(child, node->childs)) {
            treeview_update_node(widget, treeview, child);
            child = child->next;
        }
    }
}

void _tgui_tree_view_internal(TGuiWidget *widget, TGuiPainter *painter) {

    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;

    TGuiTreeView *treeview = tgui_widget_get_state(id, TGuiTreeView);
    treeview->dim = calculate_widget_rect(widget); 
    
    TGuiRectangle fake_treeview_dim = treeview_get_fake_dim(treeview, window);

    tgui_calculate_hot_widget(window, fake_treeview_dim, id);

    tgui_u32 color = TGUI_TREEVIEW_COLOR0;
    TGuiTreeViewNode *node = treeview->root->childs->next;
    while(!tgui_clink_list_end(node, treeview->root->childs)) {
        treeview_translate_node(treeview, node);
        treeview_update_node(widget, treeview, node);
        treeview_node_draw(widget, treeview, node, painter, &color);
        node = node->next; 
    }
}

TGuiRectangle calculate_option_rect(tgui_s32 x, tgui_s32 y, tgui_s32 option_index) {
    return tgui_rect_from_wh(x, y + TGUI_DROPDOWN_MENU_DELFAUT_H*(option_index + 1), TGUI_DROPDOWN_MENU_DELFAUT_W, TGUI_DROPDOWN_MENU_DELFAUT_H);
}

tgui_b32 mouse_in_window_scrollbar(TGuiWindow *window, tgui_s32 x, tgui_s32 y) {
    tgui_b32 result = false;

    tgui_b32 v_scroll_valid = !tgui_rect_invalid(window->v_scroll_bar);
    tgui_b32 h_scroll_valid = !tgui_rect_invalid(window->h_scroll_bar);
    
    if(v_scroll_valid) {
        result = result || tgui_rect_point_overlaps(window->v_scroll_bar, x, y);
    }

    if(h_scroll_valid) {
        result = result || tgui_rect_point_overlaps(window->h_scroll_bar, x, y);
    }

    return result;
}

void _tgui_dropdown_menu(TGuiWindowHandle handle, tgui_s32 x, tgui_s32 y, char **options, tgui_u32 options_size, tgui_s32 *selected_option, char *tgui_id) {

    TGuiWindow *window = tgui_window_get_from_handle(handle);
    
    tgui_u64 id = tgui_get_widget_id(tgui_id);

    if(!tgui_window_update_widget(window)) {
        if(state.active == id) {
            state.active = 0;
        }
        return;
    }
    
    TGuiDropDownMenu *dropdown = tgui_widget_get_state(id, TGuiDropDownMenu);

    dropdown->options = options;
    dropdown->options_size = options_size;
    dropdown->selected_option = selected_option;

    tgui_u32 dropdown_w = TGUI_DROPDOWN_MENU_DELFAUT_W;
    tgui_u32 dropdown_h = TGUI_DROPDOWN_MENU_DELFAUT_H;

    if(state.active == id) {
        dropdown_h += options_size * dropdown_h;
    }

    tgui_widget_alloc_into_window(id, _tgui_dropdown_menu_internal, window, x, y, dropdown_w, dropdown_h);
}

void _tgui_dropdown_menu_internal(TGuiWidget *widget, TGuiPainter *painter) {
    
    TGuiWindow *window = widget->parent;
    tgui_u64 id = widget->id;
    
    TGuiRectangle rect = calculate_widget_rect(widget);
    TGuiDropDownMenu *dropdown = tgui_widget_get_state(id, TGuiDropDownMenu);
    
    tgui_calculate_hot_widget(window, rect, id);

    tgui_b32 mouse_in_node = tgui_rect_point_overlaps(rect, input.mouse_x, input.mouse_y);

    if(state.hot == id) {
        if(mouse_in_node && input.mouse_button_is_down && !input.mouse_button_was_down) {
            state.active = id;
        }
    }
    
    if(state.active == id) {

        if(input.mouse_button_is_down && !input.mouse_button_was_down) {
            if(mouse_in_window_scrollbar(window, input.mouse_x, input.mouse_y)) {
                dropdown->click_was_in_scrollbar = true;
            } else {
                dropdown->click_was_in_scrollbar = false;
            }
        }

        if(!mouse_in_node && !dropdown->click_was_in_scrollbar && !input.mouse_button_is_down && input.mouse_button_was_down) {
            state.active = 0;
        }

        if(mouse_in_node && !input.mouse_button_is_down && input.mouse_button_was_down && state.hot == id) {

            for(tgui_u32 i = 0; i < dropdown->options_size; ++i) {
                TGuiRectangle option_rect = calculate_option_rect(rect.min_x, rect.min_y, i);
                if(tgui_rect_point_overlaps(option_rect, input.mouse_x, input.mouse_y)) {
                    *dropdown->selected_option = i;
                    state.active = 0;
                    break;
                }
            }
        }

    }

    TGuiRectangle header_rect = tgui_rect_from_wh(rect.min_x, rect.min_y, TGUI_DROPDOWN_MENU_DELFAUT_W, TGUI_DROPDOWN_MENU_DELFAUT_H);
    
    TGuiRectangle saved_painter_clip = painter->clip;
    TGuiRenderBuffer *saved_render_buffer = painter->render_buffer;

    painter->clip = tgui_rect_intersection(rect, painter->clip);
    painter->render_buffer = &state.render_state.render_buffer_tgui_on_top;

    tgui_u32 font_color = 0xffffff;

    if(state.active == id) {
        for(tgui_u32 i = 0; i < dropdown->options_size; ++i) {
            TGuiRectangle option_rect = calculate_option_rect(rect.min_x, rect.min_y, i);

            char *label = dropdown->options[i];
            TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);
            
            tgui_s32 label_x = option_rect.min_x + (tgui_rect_width(option_rect) - 1) / 2 - (tgui_rect_width(label_rect) - 1) / 2;
            tgui_s32 label_y = option_rect.min_y + (tgui_rect_height(option_rect) - 1) / 2 - (tgui_rect_height(label_rect) - 1) / 2;

            tgui_u32 color = 0x444444;
            if(state.hot == id && tgui_rect_point_overlaps(option_rect, input.mouse_x, input.mouse_y)) {
                color = 0x555555;
            }
            tgui_painter_draw_rectangle(painter, option_rect, color);
            tgui_painter_draw_hline(painter, option_rect.max_y, option_rect.min_x, option_rect.max_x, 0x888888);
            tgui_font_draw_text(painter, label_x, label_y, label, strlen(label), font_color);
        }
    }
    
    tgui_u32 cruz_w = 8;

    TGuiRectangle option_rect = header_rect;
    char *label = dropdown->options[*dropdown->selected_option];
    TGuiRectangle label_rect = tgui_get_text_dim(0, 0, label);

    tgui_s32 label_x = option_rect.min_x + (tgui_rect_width(option_rect) - 1) / 2 - (tgui_rect_width(label_rect) - 1) / 2 - cruz_w;
    tgui_s32 label_y = option_rect.min_y + (tgui_rect_height(option_rect) - 1) / 2 - (tgui_rect_height(label_rect) - 1) / 2;
    
    tgui_painter_draw_rectangle(painter, header_rect, 0x333333);
    tgui_painter_draw_rectangle_outline(painter, rect, 0x888888);
    tgui_font_draw_text(painter, label_x, label_y, label, strlen(label), font_color);

    tgui_s32 cruz_x = rect.max_x - (cruz_w + 12);
    tgui_s32 cruz_y = option_rect.min_y + (tgui_rect_height(option_rect) - 1) / 2 - (cruz_w - 1) / 2;

    TGuiRectangle cruz_rect = tgui_rect_from_wh(cruz_x, cruz_y, cruz_w, cruz_w);
    if(state.active != id) {
        TGuiRectangle v_rect = cruz_rect;
        v_rect.min_x += cruz_w / 4;
        v_rect.max_x -= cruz_w / 4;
        tgui_painter_draw_rectangle(painter, v_rect, 0x888888);
        TGuiRectangle h_rect = cruz_rect;
        h_rect.min_y += cruz_w / 4;
        h_rect.max_y -= cruz_w / 4;
        tgui_painter_draw_rectangle(painter, h_rect, 0x888888);
    } else {
        TGuiRectangle h_rect = cruz_rect;
        h_rect.min_y += cruz_w / 4;
        h_rect.max_y -= cruz_w / 4;
        tgui_painter_draw_rectangle(painter, h_rect, 0x888888);
    }

    //printf("hot widget:%lld, widget id:%lld\n", state.hot, id);
    //painter_draw_rectangle(painter, rect, 0xff00ff);
    
    painter->clip = saved_painter_clip;
    painter->render_buffer = saved_render_buffer;
}

/* ---------------------- */
/*       TGui Window      */
/* ---------------------- */

tgui_b32 tgui_window_flag_is_set(TGuiWindow *window, TGuiWindowFlags flags) {
    return window->flags & flags;
}

void tgui_window_flag_set(TGuiWindow *window, TGuiWindowFlags flag) {
    window->flags |= flag;
}

void tgui_window_flag_clear(TGuiWindow *window, TGuiWindowFlags flag) {
    window->flags &= ~flag;
}

int tgui_window_width(TGuiWindowHandle handle) {
    TGuiWindow *window = tgui_window_get_from_handle(handle);
    return tgui_rect_width(window->dim);
}

int tgui_window_height(TGuiWindowHandle handle) {
    TGuiWindow *window = tgui_window_get_from_handle(handle);
    return tgui_rect_height(window->dim);
}


tgui_b32 tgui_window_is_grabbing() {
    return docker.grabbing_window; 
}

TGuiWindow *tgui_window_alloc(TGuiDockerNode *parent, char *name, TGuiWindowFlags flags, TGuiAllocatedWindow *list) {

    TGuiAllocatedWindow *allocated_window_node = tgui_allocated_window_node_alloc();
    tgui_clink_list_insert_back(list, allocated_window_node);

    TGuiWindow *window = &allocated_window_node->window;
    TGUI_ASSERT(window);

    window->id = state.window_id_generator++;
    tgui_docker_window_node_add_window(parent, window);
    window->name =  name;
    parent->active_window = window->id;
    
    window->widgets = tgui_widget_alloc();
    tgui_clink_list_init(window->widgets);

    window->flags = flags;

    window->h_scroll_bar = tgui_rect_set_invalid();
    window->v_scroll_bar = tgui_rect_set_invalid();
    
    return window;
}

TGuiAllocatedWindow *tgui_allocated_window_get_from_handle(TGuiWindowHandle handle) {
    TGuiAllocatedWindow *allocated_window = state.allocated_windows->next;
    while(!tgui_clink_list_end(allocated_window, state.allocated_windows)) {
        TGuiWindow *window = &allocated_window->window;
        if(window->id == handle) return allocated_window;
        allocated_window = allocated_window->next;
    }
    TGUI_ASSERT(!"Invalid code path");
    return NULL;
}

TGuiWindow *tgui_window_get_from_handle(TGuiWindowHandle handle) {
    TGuiAllocatedWindow *allocated_window = tgui_allocated_window_get_from_handle(handle);
    return &allocated_window->window;
}

TGuiWindowHandle tgui_create_root_window(char *name, tgui_b32 scroll) {
    TGuiDockerNode *window_node = window_node_alloc(0);
    tgui_docker_set_root_node(window_node);
    TGuiWindow *window = tgui_window_alloc(window_node, name, scroll, state.allocated_windows);
    TGUI_ASSERT(window);
    return window->id;
}

TGuiWindowHandle tgui_split_window(TGuiWindowHandle handle, TGuiSplitDirection dir, char *name, tgui_b32 scroll) {
    TGuiWindow *window = tgui_window_get_from_handle(handle);

    TGuiDockerNode *window_node = window->parent; 
    TGUI_ASSERT(window_node->type == TGUI_DOCKER_NODE_WINDOW);

    TGuiDockerNode *new_window_node = window_node_alloc(window_node->parent);
    tgui_docker_node_split(window_node, dir, TGUI_POS_FRONT, new_window_node);
    
    TGuiWindow *new_window = tgui_window_alloc(new_window_node, name, scroll, state.allocated_windows);
    TGUI_ASSERT(new_window);
    return new_window->id;
}

void tgui_window_set_transparent(TGuiWindowHandle handle, tgui_b32 state) {
    TGuiWindow *window = tgui_window_get_from_handle(handle);
    if(state == true) {
        tgui_window_flag_set(window, TGUI_WINDOW_TRANSPARENT);
    } else {
        tgui_window_flag_clear(window, TGUI_WINDOW_TRANSPARENT);
    }
}

/* ---------------------- */
/*       TGui Fuction     */
/* ---------------------- */

void tgui_initialize(tgui_s32 window_w, tgui_s32 window_h, TGuiGfxBackend *gfx) {

    tgui_os_initialize();
    
    memset(&state, 0, sizeof(TGui));

    tgui_arena_initialize(&state.arena, 0, TGUI_ARENA_TYPE_VIRTUAL);
    tgui_virtual_map_initialize(&state.registry);

    input.window_resize = true;
    input.resize_w = window_w;
    input.resize_h = window_h;
    
    state.active_window = NULL;
    state.active_id = -1;

    state.window_id_generator = 0;

    state.free_windows = NULL;
    state.allocated_windows = tgui_allocated_window_node_alloc();
    tgui_clink_list_init(state.allocated_windows);
    
    /* ---------------------------------------------- */

    tgui_render_state_initialize(&state.render_state, gfx);
    
    state.default_texture_atlas = tgui_arena_push_struct(&state.arena, TGuiTextureAtlas, 8);
    tgui_texture_atlas_initialize(state.default_texture_atlas);

    state.default_program = gfx->create_program("./data/shaders/quadVert.hlsl", "./data/shaders/quadFrag.hlsl");

    tgui_font_initilize(&state.arena);
    tgui_docker_initialize();

}

void tgui_terminate(void) {
    
    if(docker.root != NULL) {
        tgui_serializer_write_docker_tree(docker.root, "./tgui.dat");
    }

    tgui_docker_terminate();
    tgui_font_terminate();
    
    TGuiGfxBackend *gfx = state.render_state.gfx;
    gfx->destroy_program(state.default_program);
    gfx->destroy_texture(state.default_texture);

    tgui_render_state_terminate(&state.render_state);

    tgui_virtual_map_terminate(&state.registry);
    tgui_arena_terminate(&state.arena);

    tgui_os_terminate();

    memset(&state, 0, sizeof(TGui));
}

static tgui_u32 tgui_allocated_window_list_size(TGuiAllocatedWindow *list) {
    tgui_u32 result = 0;
    TGuiAllocatedWindow *allocated_window = list->next;
    while(!tgui_clink_list_end(allocated_window, list)) {
        result += 1;
        allocated_window = allocated_window->next;
    }
    return result;
}

static tgui_b32 tgui_allocated_window_equals(TGuiAllocatedWindow *allocated_window0, TGuiAllocatedWindow *allocated_window1) {

    if(allocated_window0->window.id != allocated_window1->window.id) {
        return false;
    }
    
    if(strcmp(allocated_window0->window.name, allocated_window1->window.name) != 0) {
        return false;
    }

    return true;
}

static tgui_b32 tgui_node_tree_valid(TGuiDockerNode *node, TGuiAllocatedWindow *allocated_windows) {
    TGUI_UNUSED(node);
    
    if(tgui_allocated_window_list_size(state.allocated_windows) != tgui_allocated_window_list_size(allocated_windows)) {
        return false;
    }

    tgui_b32 window_found = false;

    TGuiAllocatedWindow *allocated_window = state.allocated_windows->next;
    while(!tgui_clink_list_end(allocated_window, state.allocated_windows)) {
        
        window_found = false;

        TGuiAllocatedWindow *allocated_window_test = allocated_windows->next;
        while(!tgui_clink_list_end(allocated_window_test, allocated_windows)) {

            if(tgui_allocated_window_equals(allocated_window, allocated_window_test)) {
                window_found =  true;
                break;
            }

            allocated_window_test = allocated_window_test->next;
        }
        
        if(window_found == false) {
            break;
        }

        allocated_window = allocated_window->next;
    }

    return window_found;
}

void tgui_free_allocated_windows_list(TGuiAllocatedWindow *list) {
    TGuiAllocatedWindow *allocated_window = list->next;
    while(!tgui_clink_list_end(allocated_window, list)) {
        TGuiAllocatedWindow *to_free = allocated_window;
        allocated_window = allocated_window->next;
        tgui_allocated_window_node_free(to_free);
    }
}

void tgui_try_to_load_data_file(void) {
    TGuiOsFile *file = tgui_os_file_read_entire("./tgui.dat");
    if(file) {
        TGuiAllocatedWindow allocated_windows;
        TGuiDockerNode *saved_root;
        tgui_serializer_read_docker_tree(file, &saved_root, &allocated_windows);

        if(tgui_node_tree_valid(saved_root, &allocated_windows)) {
            tgui_free_allocated_windows_list(state.allocated_windows);
            
            allocated_windows.next->prev = state.allocated_windows;
            allocated_windows.prev->next = state.allocated_windows;
            state.allocated_windows->next = allocated_windows.next;
            state.allocated_windows->prev = allocated_windows.prev;
            
            docker.root = saved_root;
        } else {
            tgui_free_allocated_windows_list(&allocated_windows);
            printf("tgui.dat file old or corrupted\n");
        }

        tgui_os_file_free(file);
    }
}

void tgui_begin(tgui_f32 dt) {
    state.dt = dt;

    input.mouse_x = TGUI_CLAMP(input.mouse_x, 0, (input.resize_w-1));
    input.mouse_y = TGUI_CLAMP(input.mouse_y, 0, (input.resize_h-1));

    tgui_docker_update();

    TGuiAllocatedWindow *allocated_window = state.allocated_windows->next;
    while(!tgui_clink_list_end(allocated_window, state.allocated_windows)) {
        
        TGuiWindow *window = &allocated_window->window;

        TGuiRectangle window_dim = tgui_docker_get_client_rect(window->parent);
        window->dim = window_dim;
        
        allocated_window = allocated_window->next;
    }
}

static inline TGuiRectangle calculate_total_widget_rect(TGuiWindow *window) {
    TGuiRectangle result = window->dim;
    TGuiWidget *widget = window->widgets->next;
    while(!tgui_clink_list_end(widget, window->widgets)) {
        result = tgui_rect_union(result, calculate_widget_rect(widget));
        widget = widget->next;
    }
    return result;
}

void tgui_scroll_window_recalculate_dim(TGuiWindow *window) {
    if(!tgui_window_flag_is_set(window, TGUI_WINDOW_SCROLLING)) return;
    
    tgui_b32 h_bar_added = false;
    tgui_b32 v_bar_added = false;
    TGuiRectangle widget_rect;
    
    TGuiRectangle last_scroll_rect = window->scroll_saved_rect;

    widget_rect = calculate_total_widget_rect(window);
    window->scroll_saved_rect = widget_rect;
    
    if(tgui_rect_width(widget_rect) > tgui_rect_width(window->dim)) {
        window->dim.max_y -= TGUI_SCROLL_BAR_SIZE;
        h_bar_added = true;
    }

    widget_rect = calculate_total_widget_rect(window);

    if(tgui_rect_height(widget_rect) > tgui_rect_height(window->dim)) {
        window->dim.max_x -= TGUI_SCROLL_BAR_SIZE;
        v_bar_added = true;

        if(!h_bar_added) {
            widget_rect = calculate_total_widget_rect(window);
            
            if(tgui_rect_width(widget_rect) > tgui_rect_width(window->dim)) {
                window->dim.max_y -= TGUI_SCROLL_BAR_SIZE;
                h_bar_added = true;
            }
        }
    }

    if(h_bar_added) {

        window->h_scroll_bar = window->dim;
        window->h_scroll_bar.min_y = window->dim.max_y + 1;
        window->h_scroll_bar.max_y = window->h_scroll_bar.min_y + TGUI_SCROLL_BAR_SIZE - 1;
       
        if(!tgui_rect_invalid(last_scroll_rect)) {
            tgui_u32 last_scroll_w = tgui_rect_width(last_scroll_rect) - tgui_rect_width(window->dim);
            tgui_u32 saved_scroll_w = tgui_rect_width(window->scroll_saved_rect) - tgui_rect_width(window->dim);
            if(tgui_rect_width(last_scroll_rect) != tgui_rect_width(window->scroll_saved_rect)) {
                window->h_scroll_offset = TGUI_CLAMP((tgui_f32)window->h_scroll_offset * (tgui_f32)last_scroll_w / (tgui_f32)saved_scroll_w, 0, 1);
            }
        }

    } else {
        window->h_scroll_offset = 0;
        window->h_scroll_bar = tgui_rect_set_invalid();
    }

    if(v_bar_added) {

        window->v_scroll_bar = window->dim;
        window->v_scroll_bar.min_x = window->dim.max_x + 1;
        window->v_scroll_bar.max_x = window->v_scroll_bar.min_x + TGUI_SCROLL_BAR_SIZE - 1;
    
        if(!tgui_rect_invalid(last_scroll_rect)) {
            tgui_u32 last_scroll_h = tgui_rect_height(last_scroll_rect) - tgui_rect_height(window->dim);
            tgui_u32 saved_scroll_h = tgui_rect_height(window->scroll_saved_rect) - tgui_rect_height(window->dim);
            if(tgui_rect_height(last_scroll_rect) != tgui_rect_height(window->scroll_saved_rect)) {
                window->v_scroll_offset = TGUI_CLAMP((tgui_f32)window->v_scroll_offset * (tgui_f32)last_scroll_h / (tgui_f32)saved_scroll_h, 0, 1);
            }
        }

    } else {
        window->v_scroll_offset = 0;
        window->v_scroll_bar = tgui_rect_set_invalid();
    }

}

void tgui_process_scroll_window(TGuiWindow *window, TGuiPainter *painter) {
    if(!tgui_window_flag_is_set(window, TGUI_WINDOW_SCROLLING)) return;
    
    TGuiRectangle saved_painter_clip = painter->clip;
    painter->clip = tgui_rect_intersection(painter->clip, tgui_docker_get_client_rect(window->parent));

    tgui_b32 v_scroll_valid = !tgui_rect_invalid(window->v_scroll_bar);
    tgui_b32 h_scroll_valid = !tgui_rect_invalid(window->h_scroll_bar);

    if(v_scroll_valid) {
        
        tgui_b32 mouse_over_bar = tgui_rect_point_overlaps(window->v_scroll_bar, input.mouse_x, input.mouse_y);
        if(mouse_over_bar && input.mouse_button_is_down && !input.mouse_button_was_down) {
            window->v_scroll_active = true;
        }

        if(window->v_scroll_active && !input.mouse_button_is_down) {
            window->v_scroll_active = false;
        }
        
        if(window->v_scroll_active) {
            tgui_f32 mouse_offset = (tgui_f32)(input.mouse_y - window->v_scroll_bar.min_y)/(tgui_f32)tgui_rect_height(window->v_scroll_bar);
            window->v_scroll_offset = TGUI_CLAMP(mouse_offset, 0, 1);
        }

        tgui_u32 handle_h = (tgui_u32)(((tgui_f32)tgui_rect_height(window->dim) / (tgui_f32)tgui_rect_height(window->scroll_saved_rect)) * tgui_rect_height(window->v_scroll_bar)); 
        
        if(tgui_rect_point_overlaps(window->dim, input.mouse_x, input.mouse_y)) {
            tgui_f32 pixels_to_scroll = 4;
            tgui_f32 scroll_in_pixels = window->v_scroll_offset * (tgui_f32)(tgui_rect_height(window->v_scroll_bar) - handle_h);
            scroll_in_pixels -= (input.wheel_delta * pixels_to_scroll);
            window->v_scroll_offset = TGUI_CLAMP(scroll_in_pixels / (tgui_f32)(tgui_rect_height(window->v_scroll_bar) - handle_h), 0, 1);
            //window->v_scroll_offset = TGUI_CLAMP(window->v_scroll_offset - input.wheel_delta * 0.02f, 0, 1);
        }
        
        TGuiRectangle handle = window->v_scroll_bar; 
        handle.min_y += window->v_scroll_offset * (tgui_rect_height(window->v_scroll_bar) - handle_h);
        handle.max_y = handle.min_y + handle_h - 1;

        tgui_painter_draw_rectangle(painter, window->v_scroll_bar, 0x333333);
        tgui_painter_draw_vline(painter, window->v_scroll_bar.min_x, window->v_scroll_bar.min_y, window->v_scroll_bar.max_y, 0x444444);

        tgui_painter_draw_rectangle(painter, handle, 0x555555);
        tgui_painter_draw_rectangle_outline(painter, handle, 0x888888);
    }

    if(h_scroll_valid) {

        tgui_b32 mouse_over_bar = tgui_rect_point_overlaps(window->h_scroll_bar, input.mouse_x, input.mouse_y);
        if(mouse_over_bar && input.mouse_button_is_down && !input.mouse_button_was_down) {
            window->h_scroll_active = true;
        }

        if(window->h_scroll_active && !input.mouse_button_is_down) {
            window->h_scroll_active = false;
        }
        
        if(window->h_scroll_active) {
            tgui_f32 mouse_offset = (tgui_f32)(input.mouse_x - window->h_scroll_bar.min_x)/(tgui_f32)tgui_rect_width(window->h_scroll_bar);
            window->h_scroll_offset = TGUI_CLAMP(mouse_offset, 0, 1);
        }
        
        tgui_u32 handle_w = (tgui_u32)(((tgui_f32)tgui_rect_width(window->dim) / (tgui_f32)tgui_rect_width(window->scroll_saved_rect)) * tgui_rect_width(window->h_scroll_bar)); 

        TGuiRectangle handle = window->h_scroll_bar; 
        handle.min_x += window->h_scroll_offset * (tgui_rect_width(window->h_scroll_bar) - handle_w);
        handle.max_x = handle.min_x + handle_w - 1;

        tgui_painter_draw_rectangle(painter, window->h_scroll_bar, 0x333333);
        tgui_painter_draw_hline(painter, window->h_scroll_bar.min_y, window->h_scroll_bar.min_x, window->h_scroll_bar.max_x, 0x444444);

        tgui_painter_draw_rectangle(painter, handle, 0x555555);
        tgui_painter_draw_rectangle_outline(painter, handle, 0x888888);
    }

    if(v_scroll_valid && h_scroll_valid) {
        TGuiRectangle window_rect = tgui_docker_get_client_rect(window->parent);
        TGuiRectangle inner_rect = (TGuiRectangle){window->dim.max_x+1, window->dim.max_y+1, window_rect.max_x, window_rect.max_y};
        tgui_painter_draw_rectangle(painter, inner_rect, 0x444444);
    }

    painter->clip = saved_painter_clip;
}

void tgui_end(void) {


    TGuiRenderBuffer *render_buffer_tgui = &state.render_state.render_buffer_tgui;
    tgui_render_buffer_set_program(render_buffer_tgui, state.default_program);
    tgui_render_buffer_set_texture(render_buffer_tgui, state.default_texture);
    tgui_render_buffer_set_texture_atlas(render_buffer_tgui, state.default_texture_atlas);

    TGuiRenderBuffer *render_buffer_tgui_on_top = &state.render_state.render_buffer_tgui_on_top;
    tgui_render_buffer_set_program(render_buffer_tgui_on_top, state.default_program);
    tgui_render_buffer_set_texture(render_buffer_tgui_on_top, state.default_texture);
    tgui_render_buffer_set_texture_atlas(render_buffer_tgui_on_top, state.default_texture_atlas);
    
    if(docker.root != NULL) {
        TGuiPainter painter;
        tgui_painter_start(&painter, TGUI_PAINTER_TYPE_HARDWARE, docker.root->dim, 0, NULL, render_buffer_tgui);

        tgui_docker_root_node_draw(&painter);

        TGuiAllocatedWindow *allocated_window = state.allocated_windows->next;
        while(!tgui_clink_list_end(allocated_window, state.allocated_windows)) { 

            TGuiWindow *window = &allocated_window->window;

            tgui_scroll_window_recalculate_dim(window);
            tgui_window_process_widgets(window, &painter);
            tgui_process_scroll_window(window, &painter);

            tgui_window_free_widgets(window);
            
            allocated_window = allocated_window->next;
        }

        tgui_docker_draw_preview(&painter);
    }

    input.mouse_button_was_down = input.mouse_button_is_down;
}

void tgui_draw_buffers(void) {
    tgui_u32 width = tgui_rect_width(docker.root->dim);
    tgui_u32 height = tgui_rect_height(docker.root->dim);

    state.render_state.gfx->set_program_width_and_height(state.default_program, width, height);
    tgui_render_state_draw_buffers(&state.render_state);
    tgui_render_state_clear_render_buffers(&state.render_state);

}


