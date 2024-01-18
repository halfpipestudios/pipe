#include "game.h"
#include "platform_manager.h" 
#include "mgr/texture_manager.h"

#include "serializer.h"
#include "tokenizer.h"

void Game::Initialize() {

    camera.Initialize(THIRD_PERSON_CAMERA);
    
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4());
    mapShader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/mapVert.hlsl",
                                                              "./data/shaders/mapFrag.hlsl");
    animShader = GraphicsManager::Get()->CreateShaderVertexSkin("./data/shaders/animVert.hlsl",
                                                                "./data/shaders/mapFrag.hlsl");
    statShader = GraphicsManager::Get()->CreateShaderVertex("./data/shaders/staticVert.hlsl",
                                                            "./data/shaders/staticFrag.hlsl");

    level.Initialize("./data/maps/levelOP.map", &camera, mapShader, statShader, animShader);

    // TODO: Remove this test
#if 1
    // NOTE: Serialize Test -----------------
    
    Serializer s;
    s.Begin();
    level.Serialize(&s);
    s.End("level.dat");
    
    // NOTE: Deserialize Test -----------------
    Level otherLevel;

    Tokenizer t;
    t.Begin("level.dat");
    otherLevel.Deserialize(&t);
    t.End();

    // ----------------------------------------
#endif

}

void Game::Terminate() {

    level.Terminate();
    GraphicsManager::Get()->DestroyShader(mapShader);
    GraphicsManager::Get()->DestroyShader(statShader);
    GraphicsManager::Get()->DestroyShader(animShader);
}

void Game::BeginFrame(f32 dt) {
    level.BeginFrame(dt);
}

void Game::EndFrame(f32 dt) {
    level.EndFrame(dt);
}


void Game::Update(f32 dt) {
    
    level.SetCamera(&camera);
    level.Update(dt);

    camera.ProcessMovement(&level.map, dt);
    camera.SetViewMatrix();
}

void Game::Render() { 
    level.Render();
}
