#include "texture_manager.h"
#include "../graphics_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureManager TextureManager::textureManager;

void TextureManager::Load(TextureBuffer *data, const char *name) {
    static char diffuse_material_path_cstr[4096];
    sprintf(diffuse_material_path_cstr, "%s%s", "./data/textures/", name); 
    stbi_set_flip_vertically_on_load(true);
    i32 w, h, n;
    u32 *bitmap = (u32 *)stbi_load(diffuse_material_path_cstr, &w, &h, &n, 4);
    Texture texture = {bitmap, w, h};
    TextureBuffer textureBuffer = GraphicsManager::Get()->CreateTextureBuffer(&texture, 1);
    stbi_image_free(bitmap);
    printf("Texture: %s loaded\n", diffuse_material_path_cstr);
    *data = textureBuffer;
}

void TextureManager::Unload(TextureBuffer *data) {
    GraphicsManager::Get()->DestroyTextureBuffer(*data);
    printf("remove texture: %llu\n", (u64)data);
}
