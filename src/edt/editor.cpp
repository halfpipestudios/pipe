#include "editor.h"

#include "level.h"
#include "serializer.h"
#include "graphics_manager.h"
#include "platform_manager.h"
#include "gizmo.h"

struct TGuiShader {
    VShader v;
    FShader f;
};

void *TGuiCreateShader(char *vert, char *frag) {
    TGuiShader *shader = (TGuiShader *)MemoryManager::Get()->AllocStaticMemory(sizeof(TGuiShader), 8);
    shader->v = GraphicsManager::Get()->CreateVShader(vert);
    shader->f = GraphicsManager::Get()->CreateFShader(frag);
    return (void *)shader;
}

void TGuiDestroyShader(void *program) {
    TGuiShader *shader = (TGuiShader *)program;
    GraphicsManager::Get()->DestroyVShader(shader->v);
    GraphicsManager::Get()->DestroyFShader(shader->f);
}

void *TGuiCreateTexture(tgui_u32 *data, tgui_u32 width, tgui_u32 height) {
    Texture texture = {};
    texture.pixels = (u32 *)data;
    texture.w = width;
    texture.h = height;
    TextureBuffer textureBuffer = GraphicsManager::Get()->CreateTextureBuffer(&texture, 1);
    return textureBuffer;
}

void TGuiDestroyTexture(void *texture) {
    TextureBuffer textureBuffer = (TextureBuffer)texture;
    GraphicsManager::Get()->DestroyTextureBuffer(textureBuffer);
}

void TGuiSetShaderWidthAndHeight(void *shader, tgui_u32 width, tgui_u32 height) {
    CBTGui *buffer = &GraphicsManager::Get()->cpuTGuiBuffer;
    ConstBuffer handle = GraphicsManager::Get()->gpuTGuiBuffer;

    buffer->res_X = width;
    buffer->res_y = height;
    GraphicsManager::Get()->UpdateConstBuffer(handle, buffer);
}

void TGuiDrawBuffers(void *shader, void *texture, TGuiVertexArray *vertex_buffer, TGuiU32Array *index_buffer) {

    GraphicsManager::Get()->SetDepthStencilState(false);

    TGuiShader *tguiShader = (TGuiShader *)shader;
    GraphicsManager::Get()->BindVShader(tguiShader->v);
    GraphicsManager::Get()->BindFShader(tguiShader->f);

    GraphicsManager::Get()->BindTextureBuffer(texture);
    GraphicsManager::Get()->Draw2DBatch((D3D112DVertex *)tgui_array_data(vertex_buffer), tgui_array_size(vertex_buffer),
                                        tgui_array_data(index_buffer), tgui_array_size(index_buffer));

    GraphicsManager::Get()->SetDepthStencilState(true);
}

static void TGuiUpdateInput(Input *input, TGuiInput *tguiInput) {

    tguiInput->mouse_button_is_down = input->MouseIsPress(MOUSE_BUTTON_L);
    //tguiInput->mouse_button_was_down = input->MouseJustPress(MOUSE_BUTTON_L);

    tguiInput->mouse_x = input->state[0].mouseX;
    tguiInput->mouse_y = input->state[0].mouseY;

    tguiInput->wheel_delta = input->state[0].wheelDelta;
    
    u32 w = PlatformManager::Get()->GetWindow()->GetWidth();
    u32 h = PlatformManager::Get()->GetWindow()->GetHeight();
 
    if(tguiInput->resize_w != w || tguiInput->resize_h != h) {
        tguiInput->window_resize = true;
        tguiInput->resize_w = w;
        tguiInput->resize_h = h;
    }
    
    u32 size = MIN(input->state[0].textSize, TGUI_MAX_TEXT_SIZE);
    for(u32 index = 0; index < size; ++index) {
        tguiInput->text[index] = input->state[0].text[index];
    }
    tguiInput->text_size = size;
    
    tguiInput->keyboard.k_r_arrow_down = input->state[0].kRightArrow;
    tguiInput->keyboard.k_l_arrow_down = input->state[0].kLeftArrow;
    tguiInput->keyboard.k_delete = 0;
    tguiInput->keyboard.k_backspace = input->state[0].kBackspace;
    tguiInput->keyboard.k_ctrl = input->KeyIsPress(KEY_CONTROL);
    tguiInput->keyboard.k_shift = input->KeyIsPress(KEY_SHIFT);
}


void Editor::Initialize(Level *level) {
    this->level = level;
    this->selectedEntity = {};
    this->paused = false;

    camera.Initialize(FREE_CAMERA);

    tguiBackend.create_program  = TGuiCreateShader;
    tguiBackend.destroy_program = TGuiDestroyShader;
    tguiBackend.create_texture  = TGuiCreateTexture;
    tguiBackend.destroy_texture = TGuiDestroyTexture;
    tguiBackend.set_program_width_and_height = TGuiSetShaderWidthAndHeight;
    tguiBackend.draw_buffers    = TGuiDrawBuffers;

    tgui_initialize(PlatformManager::Get()->GetWindow()->GetWidth(), 
                    PlatformManager::Get()->GetWindow()->GetHeight(),
                    &tguiBackend);
    
    tgui_texture_atlas_generate_atlas();
    
    GizmoManager::Get()->Initialize(&gameWindow, &camera);

    gameWindow.Initialize("Game", TGUI_WINDOW_TRANSPARENT, nullptr, (TGuiSplitDirection)0);
    toolWindow.Initialize("Tools", TGUI_WINDOW_SCROLLING, &gameWindow, TGUI_SPLIT_DIR_VERTICAL);
    entiWindow.Initialize("Entities", TGUI_WINDOW_SCROLLING, &toolWindow, TGUI_SPLIT_DIR_HORIZONTAL);
    compWindow.Initialize("Components", TGUI_WINDOW_SCROLLING, &entiWindow, TGUI_SPLIT_DIR_HORIZONTAL);

    tgui_try_to_load_data_file();
}

void Editor::Terminate() {
    
    gameWindow.Terminate();
    toolWindow.Terminate();
    entiWindow.Terminate();
    compWindow.Terminate();

    GizmoManager::Get()->Terminate();

    tgui_terminate();
}

void Editor::Update(f32 dt) {

    level->SetCamera(&camera);
    
    TGuiUpdateInput(PlatformManager::Get()->GetInput(), tgui_get_input());
    
    if(PlatformManager::Get()->GetInput()->KeyJustPress(KEY_0)) {
        printf("Saving level!!\n");
        Serializer s;
        s.Begin();
        level->Serialize(&s);
        s.End("./data/levels/level.dat");
    }

    if(selectedEntity) {
        GizmoManager::Get()->UpdateInput();
    }
    
    tgui_begin(dt);

    gameWindow.Update(this, dt);
    compWindow.Update(this, dt);
    entiWindow.Update(this, dt);
    toolWindow.Update(this, dt);

    tgui_end();

    camera.ProcessMovement(&gameWindow, dt);
    camera.SetViewMatrix();
}

void Editor::Render() {

    gameWindow.Render(this);
    compWindow.Render(this);
    entiWindow.Render(this);
    toolWindow.Render(this);
    
    u32 w = PlatformManager::Get()->GetWindow()->GetWidth();
    u32 h = PlatformManager::Get()->GetWindow()->GetHeight();
    
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)w/(f32)h, 0.01f, 1000.0f));
    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_POINT);
    GraphicsManager::Get()->SetViewport(0, 0, PlatformManager::Get()->GetWindow()->GetWidth(), PlatformManager::Get()->GetWindow()->GetHeight());
    GraphicsManager::Get()->BindFrameBuffer(nullptr);
    GraphicsManager::Get()->ClearColorBuffer(nullptr, 1, 0, 1);
    
    tgui_draw_buffers();
}
