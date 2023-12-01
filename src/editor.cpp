#include "editor.h"
#include "game.h"

void *TGuiCreateShader(char *vert, char *frag) {
    Shader shader = GraphicsManager::Get()->CreateShaderTGui(vert, frag);
    return (void *)shader;
}

void TGuiDestroyShader(void *program) {
    Shader shader = (Shader)program;
    GraphicsManager::Get()->DestroyShader(shader);
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

    GraphicsManager::Get()->BindShader(shader);
    GraphicsManager::Get()->BindTextureBuffer(texture);
    GraphicsManager::Get()->Draw2DBatch((D3D112DVertex *)tgui_array_data(vertex_buffer), tgui_array_size(vertex_buffer),
                                        tgui_array_data(index_buffer), tgui_array_size(index_buffer));

    GraphicsManager::Get()->SetDepthStencilState(true);
}

static void TGuiUpdateInput(Input *input, TGuiInput *tguiInput) {

    tguiInput->mouse_button_is_down = input->MouseIsPress(MOUSE_BUTTON_L);
    tguiInput->mouse_button_was_down = input->MouseJustPress(MOUSE_BUTTON_L);

    tguiInput->mouse_x = input->state[0].mouseX;
    tguiInput->mouse_y = input->state[0].mouseY;
    
    u32 w = PlatformManager::Get()->GetWindow()->GetWidth();
    u32 h = PlatformManager::Get()->GetWindow()->GetHeight();
 
    if(tguiInput->resize_w != w || tguiInput->resize_h != h) {
        tguiInput->window_resize = true;
        tguiInput->resize_w = w;
        tguiInput->resize_h = h;
    }
    
    //tguiInput->text[TGUI_MAX_TEXT_SIZE];
    tguiInput->text_size = 0;
    
    tguiInput->keyboard.k_r_arrow_down = 0;
    tguiInput->keyboard.k_l_arrow_down = 0;
    tguiInput->keyboard.k_delete = 0;
    tguiInput->keyboard.k_backspace = 0;
    tguiInput->keyboard.k_ctrl = 0;
    tguiInput->keyboard.k_shift = 0;
}


void Editor::Initialize(Game *game) {
    this->game = game;

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


    gameWindow = tgui_create_root_window("Game", TGUI_WINDOW_TRANSPARENT);
    toolWindow = tgui_split_window(gameWindow, TGUI_SPLIT_DIR_VERTICAL, "Tools", TGUI_WINDOW_SCROLLING);

    tgui_try_to_load_data_file();
    
    gameFrameBuffer = GraphicsManager::Get()->CreateFrameBuffer(0, 0, 1280, 720);
}

void Editor::Terminate() {
    GraphicsManager::Get()->DestroyFrameBuffer(gameFrameBuffer);
    tgui_terminate();
}

void Editor::Update(f32 dt) {
    game->Update(dt);

    TGuiUpdateInput(PlatformManager::Get()->GetInput(), tgui_get_input());
    
    tgui_begin(dt);
    tgui_texture(gameWindow, (void *)GraphicsManager::Get()->FrameBufferGetTexture(gameFrameBuffer));
    tgui_end();
}

void Editor::Render() {

    i32 tguiWindowW = tgui_window_width(gameWindow); 
    i32 tguiWindowH = tgui_window_height(gameWindow); 
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)tguiWindowW/(f32)tguiWindowH, 0.01f, 1000.0f));

    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_LINEAR);
    GraphicsManager::Get()->SetViewport(0, 0, 1280, 720);
    GraphicsManager::Get()->BindFrameBuffer(gameFrameBuffer);
    GraphicsManager::Get()->ClearColorBuffer(gameFrameBuffer, 0.1f, 0.05f, 0.4f);
    GraphicsManager::Get()->ClearDepthStencilBuffer(gameFrameBuffer);

    game->Render();
    GraphicsManager::Get()->FlushFrameBuffer(gameFrameBuffer);

    
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, 
                                                           (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                                                           (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                                                           0.01f, 1000.0f));

    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_POINT);
    GraphicsManager::Get()->SetViewport(0, 0, PlatformManager::Get()->GetWindow()->GetWidth(), PlatformManager::Get()->GetWindow()->GetHeight());
    GraphicsManager::Get()->BindFrameBuffer(nullptr);
    GraphicsManager::Get()->ClearColorBuffer(nullptr, 1, 0, 1);
    GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);
    
    tgui_draw_buffers();
}
