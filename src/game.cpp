#include "game.h"
#include "platform_manager.h" 

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
    
    // Shaders for the particle system
    soShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/streamOutputVert.hlsl", "./data/shaders/streamOutputFrag.hlsl");
    soGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/streamOutputGeo.hlsl");
    drawShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/drawVert.hlsl", "./data/shaders/drawFrag.hlsl");
    drawGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/drawGeo.hlsl");

    particleSystem = GraphicsManager::Get()->CreateParticleSystem(10000, soShader, soGeoShader, drawShader, drawGeoShader);

    level.Initialize("./data/maps/levelOP.map", &camera, mapShader, statShader, animShader);
}

void Game::Terminate() {
    level.Terminate();
    GraphicsManager::Get()->DestroyShader(mapShader);
    GraphicsManager::Get()->DestroyShader(statShader);
    GraphicsManager::Get()->DestroyShader(animShader);

    GraphicsManager::Get()->DestroyShader(soShader);
    GraphicsManager::Get()->DestroyGeometryShader(soGeoShader);
    GraphicsManager::Get()->DestroyShader(drawShader);
    GraphicsManager::Get()->DestroyGeometryShader(drawGeoShader);

    GraphicsManager::Get()->DestroyParticleSystem(particleSystem);
}

void Game::BeginFrame(f32 dt) {
    level.BeginFrame(dt);
}

void Game::EndFrame(f32 dt) {
    level.EndFrame(dt);
}


#include "cmp/transform_cmp.h"

void Game::Update(f32 dt) {
    
    if(dt > 0.0f) gameTime += dt;

    level.SetCamera(&camera);
    level.Update(dt);
    camera.ProcessMovement(&level.map, dt);
     
    if(dt > 0.0f) {
        TransformCMP *heroTransform = level.em.GetComponent<TransformCMP>(level.heroKey);
        GraphicsManager::Get()->UpdateParticleSystem(particleSystem, heroTransform->pos, camera.pos, gameTime, dt);
    }

    if(PlatformManager::Get()->GetInput()->KeyJustPress(KEY_P)) {
        GraphicsManager::Get()->ResetParticleSystem(particleSystem);
    }

    camera.SetViewMatrix();

}


void Game::Render() { 
    level.Render();
    GraphicsManager::Get()->RenderParticleSystem(particleSystem);
}
