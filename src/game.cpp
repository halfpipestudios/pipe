#include "game.h"
#include "platform_manager.h" 

void Game::Initialize() {
    
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

    level.Initialize("./data/maps/levelOP.map", statShader, animShader);

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
    level.Update(dt);
}


void Game::Render() { 
    level.Render(mapShader);
}


// -----------------------------------------------------------------------
void Game::FixUpdate(f32 dt) {
    // TODO: ...
}

void Game::PostUpdate(f32 t) {
    // TODO: ...
}
