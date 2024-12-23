#include "tgui_os.h"

#include "tgui_memory.h"
#include "stb_truetype.h"

#include <stdlib.h>
#include <windows.h>

static inline void tgui_os_error(void) {
    printf("OS Error: %s\n", strerror(errno));
    TGUI_ASSERT(!"Fatal error!");
}

static tgui_u64 g_os_page_size = 0;

void tgui_os_initialize(void) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    g_os_page_size = (tgui_u64)systemInfo.dwPageSize;; 
}

void tgui_os_terminate(void) {

}

/* -------------------------
       File Manager 
   ------------------------- */

TGuiOsFile *tgui_os_file_read_entire(const char *path) {
    TGuiOsFile *result = malloc(sizeof(TGuiOsFile));

    FILE *file = fopen((char *)path, "rb");
    if(!file) {
        printf("Cannot load file: %s\n", path);
        free(result);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    tgui_u64 file_size = (tgui_u64)ftell(file);
    fseek(file, 0, SEEK_SET);

    result = (TGuiOsFile *)malloc(sizeof(TGuiOsFile) + file_size+1);
    
    result->data = result + 1;
    result->size = file_size;

    TGUI_ASSERT(((tgui_u64)result->data % 8) == 0);
    fread(result->data, file_size+1, 1, file);
    ((tgui_u8 *)result->data)[file_size] = '\0';
    
    fclose(file);

    return result;
}

void tgui_os_file_free(TGuiOsFile *file) {
    free(file);
}

/* -------------------------
        Font Rasterizer 
   ------------------------- */

typedef struct TGuiOsFont {
    TGuiOsFile *file;
    stbtt_fontinfo info;
    tgui_u32 size;
    tgui_f32 size_ratio;
} TGuiOsFont;

struct TGuiOsFont *tgui_os_font_create(struct TGuiArena *arena, const char *path, tgui_u32 size) {
    TGuiOsFont *font = tgui_arena_push_struct(arena, TGuiOsFont, 8);
    font->size = size;
    font->file = tgui_os_file_read_entire(path);
    stbtt_InitFont(&font->info, font->file->data, stbtt_GetFontOffsetForIndex(font->file->data,0));
    font->size_ratio = stbtt_ScaleForPixelHeight(&font->info, size);
    return font;
}

void tgui_os_font_destroy(struct TGuiOsFont *font) {
    tgui_os_file_free(font->file);
}

void tgui_os_font_rasterize_glyph(struct TGuiOsFont *font, tgui_u32 codepoint, void **buffer, tgui_s32 *w, tgui_s32 *h, tgui_s32 *bpp) {
   *buffer = stbtt_GetCodepointBitmap(&font->info, 0, font->size_ratio, codepoint, w, h, 0,0);
   *bpp = 1;
}

tgui_s32 tgui_os_font_get_kerning_between(struct TGuiOsFont *font, tgui_u32 codepoint0, tgui_u32 codepoint1) {
    return stbtt_GetCodepointKernAdvance(&font->info, codepoint0, codepoint1) * font->size_ratio;
}

void tgui_os_font_get_vmetrics(struct TGuiOsFont *font, tgui_s32 *ascent, tgui_s32 *descent, tgui_s32 *line_gap) { 
    stbtt_GetFontVMetrics(&font->info, ascent, descent, line_gap);
    *ascent   *= font->size_ratio;
    *descent  *= font->size_ratio;
    *line_gap *= font->size_ratio;
}

void tgui_os_font_get_glyph_metrics(struct TGuiOsFont *font, tgui_u32 codepoint, tgui_s32 *adv_width, tgui_s32 *left_bearing, tgui_s32 *top_bearing) {
    stbtt_GetCodepointHMetrics(&font->info, codepoint, adv_width, left_bearing);
    *adv_width    *= font->size_ratio;
    *left_bearing *= font->size_ratio;
    tgui_s32 x0, y0, x1, y1;
    stbtt_GetCodepointBox(&font->info, codepoint, &x0, &y0, &x1, &y1);
    y1 *= font->size_ratio;
    *top_bearing = y1;
}

/* -------------------
        Memory
   ------------------- */

tgui_u64 tgui_os_get_page_size(void) {
    TGUI_ASSERT(g_os_page_size > 0);
    return g_os_page_size;
}

void *tgui_os_virtual_reserve(tgui_u64 size) {
    TGUI_ASSERT(TGUI_IS_POWER_OF_TWO(tgui_os_get_page_size()));
    TGUI_ASSERT((size & (tgui_os_get_page_size() - 1)) == 0);
    
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    if(result == 0) {
        tgui_os_error();
    }
    return result;
}

void tgui_os_virtual_commit(void *ptr, tgui_u64 size) {
    TGUI_ASSERT(TGUI_IS_POWER_OF_TWO(tgui_os_get_page_size()));
    TGUI_ASSERT(((tgui_u64)ptr & (tgui_os_get_page_size() - 1)) == 0);
    TGUI_ASSERT((size & (tgui_os_get_page_size() - 1)) == 0);
    
    void *result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    if(result == 0) {
        tgui_os_error();
    }
}

void tgui_os_virtual_decommit(void *ptr, tgui_u64 size) {
    TGUI_ASSERT(TGUI_IS_POWER_OF_TWO(tgui_os_get_page_size()));
    TGUI_ASSERT(((tgui_u64)ptr & (tgui_os_get_page_size() - 1)) == 0);
    TGUI_ASSERT((size & (tgui_os_get_page_size() - 1)) == 0);

    BOOL result = VirtualFree(ptr, size, MEM_DECOMMIT);
    if(result == 0) {
        tgui_os_error();
    }
}

void tgui_os_virtual_release(void *ptr, tgui_u64 size) {
    TGUI_ASSERT(TGUI_IS_POWER_OF_TWO(tgui_os_get_page_size()));
    TGUI_ASSERT(((tgui_u64)ptr & (tgui_os_get_page_size() - 1)) == 0);
    TGUI_ASSERT((size & (tgui_os_get_page_size() - 1)) == 0);
    
    BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
    if(result == 0) {
        tgui_os_error();
    }
}

