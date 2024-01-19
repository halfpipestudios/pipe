#include "d3d11_particle_sys.h"

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include <stdio.h>

void D3D11ParticleSystem::CreateRandomTexture(ID3D11Device *device) {
    // Create random Data
    Vec4 randomValues[1024];

    for(i32 i = 0; i < ARRAY_LENGTH(randomValues); ++i) {
        randomValues[i].x = RandF32(-1, 1);
        randomValues[i].y = RandF32(-1, 1);
        randomValues[i].z = RandF32(-1, 1);
        randomValues[i].w = RandF32(-1, 1);
    }

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
    initData.SysMemPitch = ARRAY_LENGTH(randomValues) * sizeof(Vec4);
    initData.SysMemSlicePitch = 0;

    // Create the texture
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = ARRAY_LENGTH(randomValues);
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

    ID3D11Texture1D *randomTex = 0;
    HRESULT result = device->CreateTexture1D(&texDesc, &initData, &randomTex);
    if(FAILED(result)) {
        printf("Error: Failed Creating Particle System Random Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // Create the resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
    viewDesc.Texture1D.MostDetailedMip = 0;

    result = device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV);
    if(FAILED(result)) {
        printf("Error: Failed Creating Particle System Random Texture Shader Resource View\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    randomTex->Release();

}

void D3D11ParticleSystem::CreateVertexBuffers(ID3D11Device *device) {
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(VertexParticle);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

    // the initial particle emitter has type 0 and age 0. the rest
    // of the particle attribute do not apply to the emitter
    VertexParticle p;
    ZeroMemory(&p, sizeof(VertexParticle));
    p.age = 0;
    p.type = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &p;
    HRESULT result = device->CreateBuffer(&vbd, &vinitData, &initVB);
    if(FAILED(result)) {
        printf("Error: Failed Creating Particle System Init Vertex Buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // Create the ping-pong buffers for stream-out and drawing.
    vbd.ByteWidth = sizeof(VertexParticle) * maxParticles;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

    result = device->CreateBuffer(&vbd, 0, &drawVB);
    if(FAILED(result)) {
        printf("Error: Failed Creating Particle System Draw Vertex Buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    result = device->CreateBuffer(&vbd, 0, &streamOutVB);
    if(FAILED(result)) {
        printf("Error: Failed Creating Particle System Stream Out Vertex Buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
}

void D3D11ParticleSystem::Initialize(ID3D11Device *device, 
                                     u32 maxParticles_,
                                     VShader soVShader_, FShader soFShader_, GeometryShader soGShader_,
                                     VShader dwVShader_, FShader dwFShader_, GeometryShader dwGShader_,
                                     ConstBuffer constBuffer_,
                                     Handle texture_) {

    maxParticles = maxParticles_;

    soVShader = soVShader_;
    soFShader = soFShader_;
    soGShader = soGShader_;

    dwVShader = dwVShader_;
    dwFShader = dwFShader_;
    dwGShader = dwGShader_;
    
    constBuffer = constBuffer_;
    texture = texture_;
    
    firstRun = true;
    
    CreateVertexBuffers(device);
    CreateRandomTexture(device);
}

void D3D11ParticleSystem::Terminate() {
    if(initVB) initVB->Release();
    if(drawVB) drawVB->Release();
    if(streamOutVB) streamOutVB->Release();
    if(randomTexSRV) randomTexSRV->Release();
}

void D3D11ParticleSystem::Reset() {
   firstRun = true;
    age = 0.0f;
}

void D3D11ParticleSystem::Update(Vec3 startPos, Vec3 cameraPos, f32 gameTime_, f32 dt) {
    eyePosW = cameraPos;
    emitPosW = startPos;
    gameTime = gameTime_;
    timeStep = dt;
    age += dt;
}

void D3D11ParticleSystem::Draw(ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
    // set constants
    CBParticle particlesData;
    particlesData.gameTime = gameTime;
    particlesData.timeStep = timeStep; 
    particlesData.eyePosW = eyePosW;
    particlesData.emitPosW = emitPosW;
    particlesData.emitDirW = emitDirW; 
    GraphicsManager::Get()->UpdateConstBuffer(constBuffer, &particlesData);

    GraphicsManager::Get()->SetDepthStencilState(false);

    deviceContext->GSSetShaderResources(0, 1, &randomTexSRV);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    GraphicsManager::Get()->BindVShader(soVShader);
    GraphicsManager::Get()->BindFShader(soFShader);
    GraphicsManager::Get()->DisablePixelShader();
    GraphicsManager::Get()->BindGeometryShader(soGShader);

    u32 stride = sizeof(VertexParticle);
    u32 offset = 0;

	// On the first pass, use the initialization VB.  Otherwise, use
	// the VB that contains the current particle list.
    if(firstRun) {
        deviceContext->IASetVertexBuffers(0, 1, &initVB, &stride, &offset);
    } else {
        deviceContext->IASetVertexBuffers(0, 1, &drawVB, &stride, &offset);
    }

	// Draw the current particle list using stream-out only to update them.  
	// The updated vertices are streamed-out to the target VB. 
    deviceContext->SOSetTargets(1, &streamOutVB, &offset);

    if(firstRun) {
        deviceContext->Draw(1, 0);
        firstRun = false;
    } else {
        deviceContext->DrawAuto();
    }

	// done streaming-out--unbind the vertex buffer
    ID3D11Buffer *bufferArray[1] = { 0 };
    deviceContext->SOSetTargets(1, bufferArray, &offset);

    // ping-pong the vertex buffers
    std::swap(drawVB, streamOutVB);
 
    GraphicsManager::Get()->SetDepthStencilWriteZeroState(true); 
    GraphicsManager::Get()->SetAdditiveBlendState(true);
    
    // Draw the update particle system we just stream out
    deviceContext->IASetVertexBuffers(0, 1, &drawVB, &stride, &offset);

    GraphicsManager::Get()->BindTextureBuffer(*TextureManager::Get()->Dereference(texture));
    GraphicsManager::Get()->BindVShader(dwVShader);
    GraphicsManager::Get()->BindFShader(dwFShader);
    GraphicsManager::Get()->BindGeometryShader(dwGShader);

    deviceContext->DrawAuto();
    
    GraphicsManager::Get()->SetDepthStencilWriteZeroState(false); 
    GraphicsManager::Get()->SetAdditiveBlendState(false);

    GraphicsManager::Get()->DisableVertexShader();
    GraphicsManager::Get()->DisablePixelShader();
    GraphicsManager::Get()->DisableGeometryShader();

}
