#include "game.h"
#include "platform_manager.h" 
#include "mgr/texture_manager.h"

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
    
    // Shaders for FIRE particle system
    soFireShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soFireVert.hlsl", "./data/shaders/soFireFrag.hlsl");
    soFireGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soFireGeo.hlsl");
    dwFireShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwFireVert.hlsl", "./data/shaders/dwFireFrag.hlsl");
    dwFireGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwFireGeo.hlsl");

    // Shaders for RAIN particle system
    soRainShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soRainVert.hlsl", "./data/shaders/soRainFrag.hlsl");
    soRainGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soRainGeo.hlsl");
    dwRainShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwRainVert.hlsl", "./data/shaders/dwRainFrag.hlsl");
    dwRainGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwRainGeo.hlsl");


    // TODO: see why 62 is the minimun number of particles that this system needs to have  to work well
    firePS = GraphicsManager::Get()->CreateParticleSystem(1000, soFireShader, soFireGeoShader, dwFireShader, dwFireGeoShader, TextureManager::Get()->GetAsset("flare.png"));
    rainPS = GraphicsManager::Get()->CreateParticleSystem(1000, soRainShader, soRainGeoShader, dwRainShader, dwRainGeoShader, TextureManager::Get()->GetAsset("raindrop1.png"));

    // Shader for RAIN particle system


    level.Initialize("./data/maps/levelOP.map", &camera, mapShader, statShader, animShader);
}

void Game::Terminate() {
    level.Terminate();
    GraphicsManager::Get()->DestroyShader(mapShader);
    GraphicsManager::Get()->DestroyShader(statShader);
    GraphicsManager::Get()->DestroyShader(animShader);

    GraphicsManager::Get()->DestroyShader(soFireShader);
    GraphicsManager::Get()->DestroyGeometryShader(soFireGeoShader);
    GraphicsManager::Get()->DestroyShader(dwFireShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwFireGeoShader);

    GraphicsManager::Get()->DestroyShader(soRainShader);
    GraphicsManager::Get()->DestroyGeometryShader(soRainGeoShader);
    GraphicsManager::Get()->DestroyShader(dwRainShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwRainGeoShader);

    GraphicsManager::Get()->DestroyParticleSystem(firePS);
    GraphicsManager::Get()->DestroyParticleSystem(rainPS);
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
        TransformCMP *heroTransform = EntityManager::Get()->GetComponent<TransformCMP>(level.heroKey);
        GraphicsManager::Get()->UpdateParticleSystem(firePS, Vec3(0, 3, 0), camera.pos, gameTime, dt);
        //GraphicsManager::Get()->UpdateParticleSystem(rainPS, Vec3(1.85f, -5.25f, 152.63f), camera.pos, gameTime, dt);
        GraphicsManager::Get()->UpdateParticleSystem(rainPS, heroTransform->pos, camera.pos, gameTime, dt);
    }

    if(PlatformManager::Get()->GetInput()->KeyJustPress(KEY_P)) {
        GraphicsManager::Get()->ResetParticleSystem(firePS);
        GraphicsManager::Get()->ResetParticleSystem(rainPS);
    }

    camera.SetViewMatrix();

}


void Game::Render() { 
    level.Render();
    GraphicsManager::Get()->RenderParticleSystem(firePS);
    GraphicsManager::Get()->RenderParticleSystem(rainPS);
}
