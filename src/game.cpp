#include "game.h"
#include "platform_manager.h" 

void Game::Initialize() {
    
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4());
    statShader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/texVert.hlsl",
                                                               "./data/shaders/texFrag.hlsl");
    animShader = GraphicsManager::Get()->CreateShaderSkinVertex("./data/shaders/animVert.hlsl",
                                                                "./data/shaders/texFrag.hlsl");
    level.Initialize("./data/maps/test.map", statShader, animShader);

}

void Game::Terminate() {
    level.Terminate();
    GraphicsManager::Get()->DestroyShader(statShader);
    GraphicsManager::Get()->DestroyShader(animShader);
}

void Game::Update(f32 dt) {
    level.Update(dt);
}


void Game::Render() { 
    level.Render(statShader, animShader);
}


// -----------------------------------------------------------------------
void Game::FixUpdate(f32 dt) {
    // TODO: ...
}

void Game::PostUpdate(f32 t) {
    // TODO: ...
}
