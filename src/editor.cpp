#include "editor.h"
#include "game.h"

#include "cmp/transform_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/ai_cmp.h"

#include "graphics_manager.h"
#include "mgr/texture_manager.h"

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
    this->paused = false;

    gizmoShader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/gizmoVert.hlsl",
                                                                "./data/shaders/gizmoFrag.hlsl");

    transformGizmoX = ModelManager::Get()->GetAsset("transform.twm");
    transformGizmoY = ModelManager::Get()->GetAsset("transform.twm");
    transformGizmoZ = ModelManager::Get()->GetAsset("transform.twm");

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
    GraphicsManager::Get()->DestroyShader(gizmoShader);
    GraphicsManager::Get()->DestroyFrameBuffer(gameFrameBuffer);
    tgui_terminate();
}

static void UpdateTransformComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    TransformCMP *transform = entity->GetComponent<TransformCMP>();
    ASSERT(transform);
    
    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = x + 10;
    x = label_x + 40;
    
    if(_tgui_separator(window, "Transform Component", *y, true, TGUI_ID)) {
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

static void UpdatePhysicsComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    PhysicsCMP *physicComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicComp);

    PhysicsState *p = &physicComp->physics;

    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = x + 10;
    x = label_x + 40;

    if(_tgui_separator(window, "Physic Component", *y, true, TGUI_ID)) {
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

static void UpdateCollisionComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    CollisionCMP *col = entity->GetComponent<CollisionCMP>();
    ASSERT(col);
    u32 w = 72;
    u32 h = 28;

    static char *options[] = {
        "Cylinder",
        "ConvexHull",
    };
    u32 optionsSize = sizeof(options)/sizeof(options[0]);

    u32 label_x = x + 10;
    x = label_x + 40;

    if(_tgui_separator(window, "Collision Component", *y, true, TGUI_ID)) {
        *y += h + 10;
        _tgui_label(window, "c =", 0x222222, label_x, *y, TGUI_ID);
        _tgui_dropdown_menu(window, x, *y, options, optionsSize, (i32 *)&col->type, TGUI_ID); 
        *y += 10;
    }
    *y += h;
}

static void UpdateGraphicComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Graphic Component", *y, false, TGUI_ID)) {
    }
    *y += h;
}

static void UpdateInputComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Input Component", *y, false, TGUI_ID)) {
    }
    *y += h;
}

static void UpdatePlayerAnimationComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Player Animation Component", *y, false, TGUI_ID)) {
    }
    *y += h;
}

static void UpdateMovingPlatformComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Moving Platform Component", *y, false, TGUI_ID)) {
    }
    *y += h;
}

static void UpdateAIComponent(TGuiWindowHandle window, Entity_ *entity, i32 x, i32 *y) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "AI Component", *y, false, TGUI_ID)) {
    }
    *y += h;
}

void Editor::Update(f32 dt) {
    
    TGuiUpdateInput(PlatformManager::Get()->GetInput(), tgui_get_input());
    
    tgui_begin(dt);
    
    // NOTE: Game window UI
    {
        if(paused) {
            _tgui_label(gameWindow, "game: paused", 0x00ff00, 4, 4, TGUI_ID);
        } else {
            _tgui_label(gameWindow, "game: playing", 0x00ff00, 4, 4, TGUI_ID);
        }
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
        tgui_button(toolWindow, "Load Level", x, y +  30 + 10);
        tgui_button(toolWindow, "Add Entity", x, y +  60 + 20);

        char *gameStateButton[] = {
            "Pause Game",
            "Play Game",
        };

        if(tgui_button(toolWindow, gameStateButton[paused], x, y + 90 + 30)) {
            paused = !paused;
        }
    }
    
    // NOTE: Entity window UI
    {
        Level *level = &game->level;
        auto& entities = level->em.GetEntities();
   
        _tgui_tree_view_begin(entiWindow, TGUI_ID);
        _tgui_tree_view_root_node_begin("Entities", nullptr);

        for(i32 i = 0; i < entities.size; ++i) {
            Entity_ *entity = &entities[i];
            _tgui_tree_view_node(entity->name, (void *)entity);
        }

        _tgui_tree_view_root_node_end();
        _tgui_tree_view_end((void **)&selectedEntity);

    }

    //  NOTE: Component window UI
    f32 value = 123.321f;
    {
        if(selectedEntity) {
            
            i32 y = 2;
            
            auto *elements = selectedEntity->componentsPtrs.elements;
            u32 elementsCount = selectedEntity->componentsPtrs.capacity;

            for(u32 i = 0; i < elementsCount; ++i) {
                auto *element = &elements[i];
                if(element->id == 0) continue;

                CMPBase *component = element->value;

                if(component->id == TransformCMP::GetID()) {
                    UpdateTransformComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == PhysicsCMP::GetID()) {
                    UpdatePhysicsComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == CollisionCMP::GetID()) {
                    UpdateCollisionComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == GraphicsCMP::GetID()) {
                    UpdateGraphicComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == InputCMP::GetID()) {
                    UpdateInputComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == AnimationCMP::GetID()) {
                    UpdatePlayerAnimationComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == MovingPlatformCMP::GetID()) {
                    UpdateMovingPlatformComponent(compWindow, selectedEntity, 10, &y);
                } else if(component->id == AiCMP::GetID()) {
                    UpdateAIComponent(compWindow, selectedEntity, 10, &y);
                } else {
                    char *compName = "Unknown Component";
                    _tgui_label(compWindow, compName, 0x222222, 10, y, compName);
                    y += 20;
                }

                

            }

        } else {
            _tgui_label(compWindow, "There is no entity selected!", 0x222222, 10, 10, TGUI_ID);
        }
    }
    
    tgui_end();

    if(paused) dt = 0; 

    game->Update(dt);
}

void Editor::RenderModel(Handle handle, Vec3 color) {
    
    CBGizmo *buffer = &GraphicsManager::Get()->cpuGizmoBuffer;
    ConstBuffer constBufferHandle = GraphicsManager::Get()->gpuGizmoBuffer;
    buffer->color = color;
    GraphicsManager::Get()->UpdateConstBuffer(constBufferHandle, buffer);

    Model *model = ModelManager::Get()->Dereference(handle);

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;
        if(mesh->indexBuffer) {
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, gizmoShader);
        } else {
            GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, gizmoShader);
        }
    }

}

void Editor::RenderEditorGizmos() {

    if(!selectedEntity) return;

    GraphicsManager::Get()->SetDepthStencilState(false);

    TransformCMP transform = *selectedEntity->GetComponent<TransformCMP>();

    Level *level = &game->level;
    Camera camera = level->camera;

    Vec3 p = transform.pos;
    Vec3 n = camera.front.Normalized();
    Vec3 o = camera.pos + n * 6;
   
    Vec3 v = transform.pos - camera.pos;

    f32 t = 0;
    if(ABS(v.Dot(n)) > 0.01f)  {
        t = (o.Dot(n) - p.Dot(n)) / v.Dot(n);
    }
    
    transform.rot = Vec3(0,0,0);
    transform.scale = Vec3(1,1,1);
    transform.pos = p + v * t; 

    GraphicsManager::Get()->SetWorldMatrix(transform.GetWorldMatrix());
    ModelManager::Get()->SetTexture(transformGizmoZ, "red.png");
    RenderModel(transformGizmoX, Vec3(1,0,0));
    
    TransformCMP transform1 = transform;
    transform1.rot.z += (f32)TO_RAD(90);

    GraphicsManager::Get()->SetWorldMatrix(transform1.GetWorldMatrix());
    ModelManager::Get()->SetTexture(transformGizmoZ, "green.png");
    RenderModel(transformGizmoY, Vec3(0,1,0));

    TransformCMP transform2 = transform;
    transform2.rot.y += (f32)TO_RAD(90);
    
    GraphicsManager::Get()->SetWorldMatrix(transform2.GetWorldMatrix());
    ModelManager::Get()->SetTexture(transformGizmoZ, "blue.png");
    RenderModel(transformGizmoZ, Vec3(0,0,1));

    GraphicsManager::Get()->SetDepthStencilState(true);
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
    RenderEditorGizmos();

    GraphicsManager::Get()->FlushFrameBuffer(gameFrameBuffer);
    
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, 
                                                           (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                                                           (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                                                           0.01f, 1000.0f));

    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_POINT);
    GraphicsManager::Get()->SetViewport(0, 0, PlatformManager::Get()->GetWindow()->GetWidth(), PlatformManager::Get()->GetWindow()->GetHeight());
    GraphicsManager::Get()->BindFrameBuffer(nullptr);
    GraphicsManager::Get()->ClearColorBuffer(nullptr, 1, 0, 1);
    //GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);
    
    tgui_draw_buffers();
}
