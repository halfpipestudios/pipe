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
    //tguiInput->mouse_button_was_down = input->MouseJustPress(MOUSE_BUTTON_L);

    tguiInput->mouse_x = input->state[0].mouseX;
    tguiInput->mouse_y = input->state[0].mouseY;
    
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


void Editor::Initialize(Game *game) {
    this->game = game;
    this->selectedEntity = nullptr;

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
    entiWindow = tgui_split_window(toolWindow, TGUI_SPLIT_DIR_HORIZONTAL, "Entities", TGUI_WINDOW_SCROLLING);
    compWindow = tgui_split_window(entiWindow, TGUI_SPLIT_DIR_HORIZONTAL, "Component", TGUI_WINDOW_SCROLLING);

    tgui_try_to_load_data_file();
    
    gameFrameBuffer = GraphicsManager::Get()->CreateFrameBuffer(0, 0, 1280, 720);
}

void Editor::Terminate() {
    GraphicsManager::Get()->DestroyFrameBuffer(gameFrameBuffer);
    tgui_terminate();
}

static void UpdateTransformComponent(TGuiWindowHandle window, Entity *entity, i32 x, i32 *y) {
    TransformComponent *transform = entity->GetComponent<TransformComponent>();
    ASSERT(transform);
    
    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = x + 10;
    x = label_x + 40;
    
    if(_tgui_separator(window, "Transform Component", *y, TGUI_ID)) {
        *y += h + 10;

        _tgui_label(window, "p =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &transform->pos.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->pos.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->pos.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);

        *y += h;

        _tgui_label(window, "r =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &transform->rot.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->rot.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->rot.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);

        *y += h;

        _tgui_label(window, "S =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &transform->scale.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->scale.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &transform->scale.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);
    }

    *y += h;
}

static void UpdatePhysicsComponent(TGuiWindowHandle window, Entity *entity, i32 x, i32 *y) {
    PhysicsComponent *physicComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicComp);

    Physics *p = &physicComp->physics;

    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = x + 10;
    x = label_x + 40;

    if(_tgui_separator(window, "Physic Component", *y, TGUI_ID)) {
        *y += h + 10;

        _tgui_label(window, "p =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &p->pos.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->pos.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->pos.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);

        *y += h;

        _tgui_label(window, "v =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &p->vel.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->vel.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->vel.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);

        *y += h;

        _tgui_label(window, "a =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_float_input(window, &p->acc.x, 0xff0000, x, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->acc.y, 0x00ff00, x+1*w+10, *y, w, TGUI_ID);
        _tgui_float_input(window, &p->acc.z, 0x0000ff, x+2*w+20, *y, w, TGUI_ID);
    }

    *y += h;
}

static void UpdateCollisionComponent(TGuiWindowHandle window, Entity *entity, i32 x, i32 *y) {
    CollisionComponent *col = entity->GetComponent<CollisionComponent>();
    ASSERT(col);

    u32 w = 72;
    u32 h = 28;

    if(_tgui_separator(window, "Collision Component", *y, TGUI_ID)) {
    }
    *y += h;
}

void Editor::Update(f32 dt) {
    game->Update(dt);
    
    Level *level = &game->level;
    Entity *entities = level->entities;

    TGuiUpdateInput(PlatformManager::Get()->GetInput(), tgui_get_input());
    
    tgui_begin(dt);
    
    // NOTE: Game window UI
    {
        tgui_texture(gameWindow, (void *)GraphicsManager::Get()->FrameBufferGetTexture(gameFrameBuffer));
    }

    // NOTE: Tools window UI
    {
        // TODO: These are default of the tgui library, make interface to get them
        u32 buttonW = 120;
        u32 buttonH = 30;

        i32 w = tgui_window_width(toolWindow); 
        i32 h = tgui_window_height(toolWindow); 
        i32 x = w / 2 - buttonW / 2; if(x < 0) x = 0;
        i32 y = 10;
        tgui_button(toolWindow, "Save Level", x, y);
        tgui_button(toolWindow, "Load Level", x, y + 30 + 10);
        tgui_button(toolWindow, "Add Entity", x, y + 60 + 20);
    }
    
    // NOTE: Entity window UI
    {
        Entity *entity = entities;

        _tgui_tree_view_begin(entiWindow, TGUI_ID);
        _tgui_tree_view_root_node_begin("Entities", nullptr);

        while(entity != nullptr) {
            _tgui_tree_view_node(entity->name, (void *)entity);
            entity = entity->next;
        }

        _tgui_tree_view_root_node_end();
        _tgui_tree_view_end((void **)&selectedEntity);

    }

    //  NOTE: Component window UI
    f32 value = 123.321f;
    {
        if(selectedEntity) {
            
            i32 y = 10;

            ComponentContainer *container = selectedEntity->componentContainerList;
            while(container != nullptr) {
                
                Component *component =  (Component *)&container->component;

                if(typeid(*component) == typeid(TransformComponent)) {
                    UpdateTransformComponent(compWindow, selectedEntity, 10, &y);
                } else if(typeid(*component) == typeid(PhysicsComponent)) {
                    UpdatePhysicsComponent(compWindow, selectedEntity, 10, &y);
                } else if(typeid(*component) == typeid(CollisionComponent)) {
                    UpdateCollisionComponent(compWindow, selectedEntity, 10, &y);
                } else {
                    char *compName = (char *)typeid(*component).name();
                    _tgui_label(compWindow, compName, 0x222222, 10, y, compName);
                    y += 20;
                }
                
                container = container->next;
            }


        } else {
            _tgui_label(compWindow, "There is no entity selected!", 0x222222, 10, 10, TGUI_ID);
        }
    }
    
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
    // GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);
    
    tgui_draw_buffers();
}
