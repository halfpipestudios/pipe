#include "tgui_gfx.h"
#include "tgui_painter.h"
#include "tgui.h"
#include <stdio.h>

extern TGui state;

TGuiBitmap tgui_bitmap_alloc_empty(TGuiArena *arena, tgui_u32 w, tgui_u32 h) {
    
    tgui_u64 bitmap_size = w*h*sizeof(tgui_u32);

    TGuiBitmap bitmap;
    bitmap.pixels  = tgui_arena_alloc(arena, bitmap_size, 8);
    bitmap.width   = w;
    bitmap.height  = h;
    bitmap.texture = NULL;
    
    memset(bitmap.pixels, 0, bitmap_size);

    return bitmap;
}

TGuiBitmap tgui_bitmap_copy(TGuiArena *arena, TGuiBitmap *bitmap) {
    
    TGuiBitmap result = tgui_bitmap_alloc_empty(arena, bitmap->width, bitmap->height);
    
    tgui_u64 bitmap_size = bitmap->width*bitmap->height*sizeof(tgui_u32);
    memcpy(result.pixels, bitmap->pixels, bitmap_size);

    result.texture = bitmap->texture;
    
    return result;
}

void tgui_bitmap_flip_verticaly(TGuiBitmap *bitmap) {
    for(tgui_s32 y = 0; y < bitmap->height/2; ++y) {
        for(tgui_s32 x = 0; x < bitmap->width; ++x) {
            tgui_u32 temp = bitmap->pixels[y*bitmap->width + x];
            bitmap->pixels[y*bitmap->width + x] = bitmap->pixels[(bitmap->height-y-1)*bitmap->width + x];
            bitmap->pixels[(bitmap->height-y-1)*bitmap->width + x] = temp;
            
        }
    }
}

void tgui_texture_atlas_initialize(TGuiTextureAtlas *texture_atlas) {
    tgui_arena_initialize(&texture_atlas->arena, 0, TGUI_ARENA_TYPE_VIRTUAL);
    tgui_array_initialize(&texture_atlas->textures);

    texture_atlas->bitmap.pixels = NULL;
    texture_atlas->bitmap.height = 0;
    texture_atlas->bitmap.width = TGUI_TEXTURE_ATLAS_START_WIDTH;

    texture_atlas->current_x = 0;
    texture_atlas->current_y = 0;
}

void tgui_texture_atlas_terminate(TGuiTextureAtlas *texture_atlas) {
    tgui_array_terminate(&texture_atlas->textures);
    tgui_arena_terminate(&texture_atlas->arena);
    memset(texture_atlas, 0, sizeof(TGuiTextureAtlas));

}

void tgui_texture_atlas_add_bitmap(TGuiTextureAtlas *texture_atlas, TGuiBitmap *bitmap) {

    TGuiTexture *texture = tgui_arena_push_struct(&state.arena, TGuiTexture, 8);
    texture->bitmap = bitmap;
    texture->dim = tgui_rect_set_invalid();
    
    TGuiTextureBucket *bucket = tgui_array_push(&texture_atlas->textures);
    bucket->texture = texture;
    
    bitmap->texture = texture;
}

void texture_atlas_sort_textures_per_height(TGuiTextureAtlas *texture_atlas) {
    
    for(tgui_u32 i = 0; i < tgui_array_size(&texture_atlas->textures); ++i) {
        
        TGuiTextureBucket *bucket = tgui_array_get_ptr(&texture_atlas->textures, i);

        for(tgui_u32 j = i; j < tgui_array_size(&texture_atlas->textures); ++j) {
            
            if(i == j) continue;

            TGuiTexture *texture = bucket->texture;
            tgui_u32 h = texture->bitmap->height;

            TGuiTextureBucket *other_bucket = tgui_array_get_ptr(&texture_atlas->textures, j);
            TGuiTexture *other_texture = other_bucket->texture;

            tgui_u32 other_h = other_texture->bitmap->height;

            if(h < other_h) {
                TGuiTextureBucket temp = *bucket;
                *bucket = *other_bucket;
                *other_bucket = temp;
            }
        }
    }
}

void texture_atlas_insert(TGuiTextureAtlas *texture_atlas, TGuiTexture *texture) {
   TGuiBitmap *texture_atlas_bitmap = &texture_atlas->bitmap;
    
    TGuiBitmap *bitmap = texture->bitmap;
    TGUI_ASSERT(bitmap);

    if(texture_atlas_bitmap->pixels == NULL) {

        texture_atlas->last_row_added_height = bitmap->height + TGUI_TEXTURE_ATLAS_DEFAULT_PADDING;
        texture_atlas_bitmap->height = texture_atlas->last_row_added_height;
        
        tgui_u64 texture_atlas_size = texture_atlas_bitmap->width*texture_atlas_bitmap->height*sizeof(tgui_u32);
        texture_atlas_bitmap->pixels = tgui_arena_alloc(&texture_atlas->arena, texture_atlas_size, 8);
        
        memset(texture_atlas_bitmap->pixels, 0, texture_atlas_size);
        
        texture_atlas_bitmap->pixels[0] = 0xffffffff;
        texture_atlas->current_x = 1 + TGUI_TEXTURE_ATLAS_DEFAULT_PADDING;
    }

    TGUI_ASSERT(texture_atlas_bitmap->pixels);
    TGUI_ASSERT(texture_atlas_bitmap->height >= bitmap->height);
    
    tgui_s32 width_left = texture_atlas_bitmap->width - texture_atlas->current_x; (void) width_left;
    if(width_left < (tgui_s32)bitmap->width) {

        printf("texture atlas was resize\n");
        
        tgui_u64 temp_arena_checkpoint = state.arena.used;
        
        TGuiBitmap old_texture_atlas_bitmap = tgui_bitmap_copy(&state.arena, &texture_atlas->bitmap);
        
        tgui_u32 new_row_height = bitmap->height + TGUI_TEXTURE_ATLAS_DEFAULT_PADDING;
        tgui_u32 new_texture_atlas_w = texture_atlas->bitmap.width;
        tgui_u32 new_texture_atlas_h = texture_atlas->bitmap.height + new_row_height;

        tgui_arena_free(&texture_atlas->arena);
        texture_atlas->bitmap = tgui_bitmap_alloc_empty(&texture_atlas->arena, new_texture_atlas_w, new_texture_atlas_h);

        TGuiPainter painter;
        TGuiRectangle texture_atlas_rect = tgui_rect_from_wh(0, 0,texture_atlas->bitmap.width, texture_atlas->bitmap.height);
        tgui_painter_start(&painter, TGUI_PAINTER_TYPE_SOFTWARE, texture_atlas_rect, 0, texture_atlas->bitmap.pixels, NULL);
        tgui_painter_draw_bitmap_no_alpha(&painter, 0, 0, &old_texture_atlas_bitmap);

        state.arena.used = temp_arena_checkpoint;
        
        texture_atlas->current_x = 0;
        texture_atlas->current_y += texture_atlas->last_row_added_height;

        texture_atlas->last_row_added_height = new_row_height;
    }

    texture_atlas_bitmap = &texture_atlas->bitmap;

    texture->dim = tgui_rect_from_wh(texture_atlas->current_x, texture_atlas->current_y, bitmap->width, bitmap->height);

    TGuiPainter painter;
    TGuiRectangle texture_atlas_rect = tgui_rect_from_wh(0, 0,texture_atlas_bitmap->width, texture_atlas_bitmap->height);
    tgui_painter_start(&painter, TGUI_PAINTER_TYPE_SOFTWARE, texture_atlas_rect, 0, texture_atlas_bitmap->pixels, NULL);
    
    tgui_painter_draw_bitmap_no_alpha(&painter, texture_atlas->current_x, texture_atlas->current_y, bitmap);
    
    texture_atlas->current_x += bitmap->width + TGUI_TEXTURE_ATLAS_DEFAULT_PADDING; 

}

void tgui_texture_atlas_generate_atlas(void) {

    TGuiTextureAtlas *texture_atlas = state.default_texture_atlas;

    printf("Sorting texture atlas textures!\n");
    texture_atlas_sort_textures_per_height(texture_atlas);

    printf("Generating texture atlas!\n");
    for(tgui_u32 i = 0; i < tgui_array_size(&texture_atlas->textures); ++i) {

        TGuiTextureBucket *bucket = tgui_array_get_ptr(&texture_atlas->textures, i);
        TGuiTexture *texture = bucket->texture;
        //printf("%d) bitmap height: %d\n", (i + 1), texture->bitmap->height);
        
        texture_atlas_insert(texture_atlas, texture);
    }

    //tgui_bitmap_flip_verticaly(&texture_atlas->bitmap);

    state.default_texture = state.render_state.gfx->create_texture(texture_atlas->bitmap.pixels, texture_atlas->bitmap.width, texture_atlas->bitmap.height);
}

tgui_u32 tgui_texture_atlas_get_width(TGuiTextureAtlas *texture_atlas) {
    return texture_atlas->bitmap.width;
}

tgui_u32 tgui_texture_atlas_get_height(TGuiTextureAtlas *texture_atlas) {
    return texture_atlas->bitmap.height;
}

/* ----------------------------------- */
/*          TGui Render Buffer         */
/* ----------------------------------- */

void tgui_render_buffer_initialize(TGuiRenderBuffer *render_buffer) {

    tgui_array_initialize(&render_buffer->vertex_buffer);
    tgui_array_initialize(&render_buffer->index_buffer);

    render_buffer->program = NULL;
    render_buffer->texture = NULL;
    render_buffer->texture_atlas = NULL;

}

void tgui_render_buffer_terminate(TGuiRenderBuffer *render_buffer) {
    tgui_array_terminate(&render_buffer->vertex_buffer);
    tgui_array_terminate(&render_buffer->index_buffer);
}

void tgui_render_buffer_clear(TGuiRenderBuffer *render_buffer) {
    tgui_array_clear(&render_buffer->vertex_buffer);
    tgui_array_clear(&render_buffer->index_buffer);
}

void tgui_render_buffer_set_program(TGuiRenderBuffer *render_buffer, void *program) {
    render_buffer->program = program;
}

void tgui_render_buffer_set_texture(TGuiRenderBuffer *render_buffer, void *texture) {
    render_buffer->texture = texture;
}

void tgui_render_buffer_set_texture_atlas(TGuiRenderBuffer *render_buffer, TGuiTextureAtlas *texture_atlas) {
    render_buffer->texture_atlas = texture_atlas;
}

/* ----------------------------------- */
/*          TGui Render State          */
/* ----------------------------------- */

void tgui_render_state_initialize(TGuiRenderState *render_state, struct TGuiGfxBackend *gfx) {

    tgui_render_buffer_initialize(&render_state->render_buffer_tgui);
    tgui_render_buffer_initialize(&render_state->render_buffer_tgui_on_top);
    
    tgui_array_initialize(&render_state->render_buffers_custom);

    render_state->gfx = gfx;
    render_state->current_render_buffer_custom_pushed_count = 0;
}

void tgui_render_state_terminate(TGuiRenderState *render_state) {

    for(tgui_u32 i = 0; i < tgui_array_size(&render_state->render_buffers_custom); ++i) {
        TGuiRenderBuffer *render_buffer = tgui_array_get_ptr(&render_state->render_buffers_custom, i);
        tgui_render_buffer_terminate(render_buffer);
    }
    tgui_array_terminate(&render_state->render_buffers_custom);

    tgui_render_buffer_terminate(&render_state->render_buffer_tgui);
    tgui_render_buffer_terminate(&render_state->render_buffer_tgui_on_top);

}

void tgui_render_state_clear_render_buffers(TGuiRenderState *render_state) {

    tgui_render_buffer_clear(&render_state->render_buffer_tgui);
    tgui_render_buffer_clear(&render_state->render_buffer_tgui_on_top);

    for(tgui_u32 i = 0; i < tgui_array_size(&render_state->render_buffers_custom); ++i) {
        TGuiRenderBuffer *render_buffer = tgui_array_get_ptr(&render_state->render_buffers_custom, i);
        tgui_render_buffer_clear(render_buffer);
    }

    render_state->current_render_buffer_custom_pushed_count = 0;

}

TGuiRenderBuffer *tgui_render_state_push_render_buffer_custom(TGuiRenderState *render_state, void *program, void *texture, TGuiTextureAtlas *texture_atlas) {
    TGuiRenderBuffer *render_buffer = NULL;
    
    if(render_state->current_render_buffer_custom_pushed_count >= tgui_array_size(&render_state->render_buffers_custom)) {
        render_buffer = tgui_array_push(&render_state->render_buffers_custom);
        tgui_render_buffer_initialize(render_buffer);
    } else {
        render_buffer = tgui_array_get_ptr(&render_state->render_buffers_custom, render_state->current_render_buffer_custom_pushed_count);
    }
    ++render_state->current_render_buffer_custom_pushed_count;

    tgui_render_buffer_set_program(render_buffer, program);
    tgui_render_buffer_set_texture(render_buffer, texture);
    tgui_render_buffer_set_texture_atlas(render_buffer, texture_atlas);

    TGUI_ASSERT(render_buffer);
    return render_buffer;
}

void tgui_render_buffer_draw(TGuiRenderState *render_state, TGuiRenderBuffer *render_buffer) {
    render_state->gfx->draw_buffers(render_buffer->program, render_buffer->texture, &render_buffer->vertex_buffer, &render_buffer->index_buffer);
}

void tgui_render_state_draw_buffers(TGuiRenderState *render_state) {

    TGUI_ASSERT(render_state->current_render_buffer_custom_pushed_count <= tgui_array_size(&render_state->render_buffers_custom));

    for(tgui_u32 i = 0; i < render_state->current_render_buffer_custom_pushed_count; ++i) {

        TGuiRenderBuffer *render_buffer = tgui_array_get_ptr(&render_state->render_buffers_custom, i);

        tgui_render_buffer_draw(render_state, render_buffer);
    }
    
    tgui_render_buffer_draw(render_state, &render_state->render_buffer_tgui);

    tgui_render_buffer_draw(render_state, &render_state->render_buffer_tgui_on_top);
}
