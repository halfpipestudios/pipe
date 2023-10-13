#include "d3d11_graphics.h"

#include "platform_manager.h"
#include "memory_manager.h"

#include <stdio.h>

D3D11ShaderStorage D3D11Graphics::shadersStorage;
D3D11ConstBufferStorage D3D11Graphics::constBufferStorage;
D3D11VertexBufferStorage D3D11Graphics::vertexBufferStorage;

void D3D11Graphics::Initialize() {

    i32 gMsaa = 4;

    i32 deviceFlags = D3D11_CREATE_DEVICE_DEBUG; //D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, 0, 0, D3D11_SDK_VERSION, &device, &featureLevel, &deviceContext);

    UINT msaaQuality4x;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, gMsaa, &msaaQuality4x);
    ASSERT(msaaQuality4x > 0);

    i32 windowWidth = PlatformManager::Get()->GetWindow()->GetWidth();
    i32 windowHeight = PlatformManager::Get()->GetWindow()->GetHeight();

    // create the d3d11 device swapchain and device context
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = gMsaa;
    swapChainDesc.SampleDesc.Quality = msaaQuality4x - 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = *((HWND *)PlatformManager::Get()->GetWindow()->GetOsWindow());
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    IDXGIDevice* dxgiDevice = 0;
    device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    IDXGIAdapter* dxgiAdapter = 0;
    dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    IDXGIFactory* dxgiFactory = 0;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
    dxgiFactory->CreateSwapChain(device, &swapChainDesc, &swapChain);
    if(dxgiDevice) dxgiDevice->Release();
    if(dxgiAdapter) dxgiAdapter->Release();
    if(dxgiFactory) dxgiFactory->Release();
    
    // create render target view
    ID3D11Texture2D *backBufferTexture = 0;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBufferTexture);
    device->CreateRenderTargetView(backBufferTexture, 0, &renderTargetView);
    if(backBufferTexture)
    {
        backBufferTexture->Release();
    }

    // set up the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)windowWidth;
    viewport.Height = (f32)windowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = windowWidth;
    depthStencilTextureDesc.Height = windowHeight;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = gMsaa;
    depthStencilTextureDesc.SampleDesc.Quality = msaaQuality4x - 1;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
    result = device->CreateTexture2D(&depthStencilTextureDesc, 0, &depthStencilTexture);
 
    result = device->CreateDepthStencilView(depthStencilTexture, 0, &depthStencilView);
    if (depthStencilTexture)
    {
        depthStencilTexture->Release();
    }

    // create depth stencil states
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Depth test parameters
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    // Stencil test parameters
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOn);
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.StencilEnable = false;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOff);

    // Alpha blending
    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = true;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDesc, &alphaBlendEnable);

    blendStateDesc.RenderTarget[0].BlendEnable = false;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDesc, &alphaBlendDisable);

    // Create Rasterizers Types
    D3D11_RASTERIZER_DESC fillRasterizerFrontDesc = {};
    fillRasterizerFrontDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerFrontDesc.CullMode = D3D11_CULL_FRONT;
    fillRasterizerFrontDesc.DepthClipEnable = true;
    fillRasterizerFrontDesc.AntialiasedLineEnable = true;
    fillRasterizerFrontDesc.MultisampleEnable = true;
    device->CreateRasterizerState(&fillRasterizerFrontDesc, &fillRasterizerCullFront);

    D3D11_RASTERIZER_DESC fillRasterizerBackDesc = {};
    fillRasterizerBackDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerBackDesc.CullMode = D3D11_CULL_BACK;
    fillRasterizerBackDesc.DepthClipEnable = true;
    fillRasterizerBackDesc.AntialiasedLineEnable = true;
    fillRasterizerBackDesc.MultisampleEnable = true;
    device->CreateRasterizerState(&fillRasterizerBackDesc, &fillRasterizerCullBack);

    D3D11_RASTERIZER_DESC fillRasterizerNoneDesc = {};
    fillRasterizerNoneDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerNoneDesc.CullMode = D3D11_CULL_NONE;
    fillRasterizerNoneDesc.DepthClipEnable = true;
    fillRasterizerNoneDesc.AntialiasedLineEnable = true;
    fillRasterizerNoneDesc.MultisampleEnable = true;
    device->CreateRasterizerState(&fillRasterizerNoneDesc, &fillRasterizerCullNone);

    D3D11_RASTERIZER_DESC wireFrameRasterizerDesc = {};
    wireFrameRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireFrameRasterizerDesc.CullMode = D3D11_CULL_NONE;
    wireFrameRasterizerDesc.DepthClipEnable = true;
    wireFrameRasterizerDesc.AntialiasedLineEnable = true;
    wireFrameRasterizerDesc.MultisampleEnable = true;
    device->CreateRasterizerState(&wireFrameRasterizerDesc, &wireFrameRasterizer);

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
    deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
    deviceContext->RSSetState(fillRasterizerCullBack);

    cpuMatrices.proj = Mat4();
    cpuMatrices.view = Mat4();
    cpuMatrices.world = Mat4();

    gpuMatrices = CreateConstBuffer((void *)&cpuMatrices, sizeof(cpuMatrices), 0, 0);
}

void  D3D11Graphics::Terminate() {
    if(device) device->Release();
    if(deviceContext) deviceContext->Release();
    if(swapChain) swapChain->Release();
    if(renderTargetView) renderTargetView->Release();
    if(depthStencilView) depthStencilView->Release();
    if(wireFrameRasterizer) wireFrameRasterizer->Release();
    if(fillRasterizerCullBack) fillRasterizerCullBack->Release();
    if(fillRasterizerCullFront) fillRasterizerCullFront->Release();
    if(fillRasterizerCullNone) fillRasterizerCullNone->Release();
    if(depthStencilOn) depthStencilOn->Release();
    if(depthStencilOff) depthStencilOff->Release();
    if(alphaBlendEnable) alphaBlendEnable->Release();
    if(alphaBlendDisable) alphaBlendDisable->Release();

    for(i32 i = 0; i < shadersStorage.shadersCount; ++i) {
        DestroyShader(i);
    }
    for(i32 i = 0; i < constBufferStorage.constBuffersCount; ++i) {
        DestroyConstBuffer(i);
    }
    for(i32 i = 0; i < vertexBufferStorage.vertexBuffersCount; ++i) {
        DestroyVertexBuffer(i);
    }
}

void  D3D11Graphics::SetRasterizerState(RasterizerState state) {
    switch(state) {
        case RASTERIZER_STATE_CULL_BACK: {
            deviceContext->RSSetState(fillRasterizerCullBack);
        } break;
        case RASTERIZER_STATE_CULL_FRONT: {
            deviceContext->RSSetState(fillRasterizerCullFront);
        } break;
        case RASTERIZER_STATE_CULL_NONE: {
            deviceContext->RSSetState(fillRasterizerCullNone);
        } break;
        case RASTERIZER_STATE_WIREFRAME: {
            deviceContext->RSSetState(wireFrameRasterizer);
        } break;
    }
}

void  D3D11Graphics::SetDepthStencilState(bool value) {
    if(value)
        deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
    else
        deviceContext->OMSetDepthStencilState(depthStencilOff, 1);
}

void D3D11Graphics::SetAlphaBlendState(bool value) {
    if(value)
        deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
    else
        deviceContext->OMSetBlendState(alphaBlendDisable, 0, 0xffffffff);
}

void D3D11Graphics::ClearColorBuffer(f32 r, f32 g, f32 b) {
    float clearColor[] = { r, g, b, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
}

void D3D11Graphics::ClearDepthStencilBuffer() {
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void D3D11Graphics::Present(i32 vsync) {
    swapChain->Present(vsync, 0);
}


Shader D3D11Graphics::CreateShader(char *vertpath, char *fragpath)
{
    Shader shaderHandle = -1;
    D3D11Shader shader = {}; 

    MemoryManager::Get()->BeginTemporalMemory();
    File vertfile = PlatformManager::Get()->ReadFileToTemporalMemory(vertpath);
    File fragfile = PlatformManager::Get()->ReadFileToTemporalMemory(fragpath);

    ID3DBlob *vertexShaderCompiled = 0;
    ID3DBlob *fragmentShaderCompiled = 0;
    
    HRESULT result = 0;
    ID3DBlob *errorVertexShader = 0;
    result = D3DCompile(vertfile.data, vertfile.size,
                        0, 0, 0, "vs_main", "vs_5_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &vertexShaderCompiled,
                        &errorVertexShader);
    if(errorVertexShader != 0) {
        char *errorString = (char *)errorVertexShader->GetBufferPointer();
        printf("error compiling vertex shader (%s): %s", vertpath, errorString);
        errorVertexShader->Release();
        ASSERT(!"INVALID_CODE_PATH");
    }

    ID3DBlob *errorFragmentShader = 0;
    result = D3DCompile(fragfile.data, fragfile.size,
                        0, 0, 0, "fs_main", "ps_5_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &fragmentShaderCompiled,
                        &errorFragmentShader);
    if(errorFragmentShader) {
        char *errorString = (char *)errorFragmentShader->GetBufferPointer();
        printf("error compiling fragment shader (%s): %s", fragpath, errorString);
        errorFragmentShader->Release();
        ASSERT(!"INVALID_CODE_PATH")
    }

    // create the vertex and fragment shader
    result = device->CreateVertexShader(
            vertexShaderCompiled->GetBufferPointer(),
            vertexShaderCompiled->GetBufferSize(), 0,
            &shader.vertex);
    result = device->CreatePixelShader(
            fragmentShaderCompiled->GetBufferPointer(),
            fragmentShaderCompiled->GetBufferSize(), 0,
            &shader.fragment);

    MemoryManager::Get()->EndTemporalMemory();


    // create input layout
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
         0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    i32 totalLayoutElements = ARRAY_LENGTH(inputLayoutDesc);
    HRESULT layoutResult = device->CreateInputLayout(inputLayoutDesc,
        totalLayoutElements,
        vertexShaderCompiled->GetBufferPointer(),
        vertexShaderCompiled->GetBufferSize(),
        &shader.layout);

    if(FAILED(layoutResult)) {
        OutputDebugString("ERROR Creating Layout Input\n");
        ASSERT(!"ERROR");
    }

    if(vertexShaderCompiled) vertexShaderCompiled->Release();
    if(fragmentShaderCompiled) fragmentShaderCompiled->Release();


    shadersStorage.shaders[shadersStorage.shadersCount] = shader;
    shaderHandle = shadersStorage.shadersCount;
    shadersStorage.shadersCount++;

    return shaderHandle;
}

void D3D11Graphics::DestroyShader(Shader shaderHandle) {
    D3D11Shader *shader = shadersStorage.shaders + shaderHandle;
    if(shader->vertex) shader->vertex->Release();
    if(shader->fragment) shader->fragment->Release();
    if(shader->layout) shader->layout->Release();
    shadersStorage.shadersCount--;
}

ConstBuffer D3D11Graphics::CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName) {
    ConstBuffer constBufferHandle = -1;
    D3D11ConstBuffer constBuffer = {};

    D3D11_BUFFER_DESC constBufferDesc;
    ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufferDesc.ByteWidth = bufferSize;
    constBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT result = device->CreateBuffer(&constBufferDesc, 0, &constBuffer.buffer);
    if(FAILED(result))
    {
        printf("error creating const buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    deviceContext->UpdateSubresource(constBuffer.buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(index, 1, &constBuffer.buffer);
    deviceContext->PSSetConstantBuffers(index, 1, &constBuffer.buffer);
    constBuffer.index = index;

    constBufferStorage.constBuffers[constBufferStorage.constBuffersCount] = constBuffer;
    constBufferHandle = constBufferStorage.constBuffersCount;
    constBufferStorage.constBuffersCount++;

    return constBufferHandle;
}

void D3D11Graphics::DestroyConstBuffer(ConstBuffer constBufferHandle) {
    D3D11ConstBuffer *constBuffer = constBufferStorage.constBuffers + constBufferHandle;
    if(constBuffer->buffer) constBuffer->buffer->Release();
    constBuffer->index = -1;
    constBufferStorage.constBuffersCount--;
}

void D3D11Graphics::UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData) {
    D3D11ConstBuffer *constBuffer = constBufferStorage.constBuffers + constBufferHandle;
    deviceContext->UpdateSubresource(constBuffer->buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
    deviceContext->PSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
}

void D3D11Graphics::SetProjMatrix(Mat4 proj) {
    cpuMatrices.proj = proj;
    UpdateConstBuffer(gpuMatrices, (void *)&cpuMatrices);
}

void D3D11Graphics::SetViewMatrix(Mat4 view) {
    cpuMatrices.view = view;
    UpdateConstBuffer(gpuMatrices, (void *)&cpuMatrices);
}

void D3D11Graphics::SetWorldMatrix(Mat4 world) {
    cpuMatrices.world = world;
    UpdateConstBuffer(gpuMatrices, (void *)&cpuMatrices);
}


VertexBuffer D3D11Graphics::CreateVertexBuffer(Vertex *vertices, u32 count) {
    VertexBuffer vertexBufferHandle = -1;

    D3D11VertexBuffer vertexBuffer = {};
    vertexBuffer.verticesCount = count;

    // create gpu buffer
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(Vertex) * count;
    resourceData.pSysMem = vertices;

    HRESULT result = device->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer.buffer);
    if(FAILED(result)) {
        printf("error loading vertex buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    vertexBufferStorage.vertexBuffers[vertexBufferStorage.vertexBuffersCount] = vertexBuffer;
    vertexBufferHandle = vertexBufferStorage.vertexBuffersCount;
    vertexBufferStorage.vertexBuffersCount++;

    return vertexBufferHandle;
}

void D3D11Graphics::DestroyVertexBuffer(VertexBuffer vertexBufferHandle) {
    D3D11VertexBuffer *vertexBuffer = vertexBufferStorage.vertexBuffers + vertexBufferHandle;
    if(vertexBuffer->buffer) vertexBuffer->buffer->Release();
    vertexBuffer->verticesCount = 0;
    vertexBufferStorage.vertexBuffersCount--;
}

void D3D11Graphics::DrawVertexBuffer(VertexBuffer vertexBufferHandle, Shader shaderHandle) {
    // set shader
    D3D11Shader *shader = shadersStorage.shaders + shaderHandle;
    deviceContext->VSSetShader(shader->vertex, 0, 0);
    deviceContext->PSSetShader(shader->fragment, 0, 0);
    deviceContext->IASetInputLayout(shader->layout);

    // set buffer
    D3D11VertexBuffer *vertexBuffer = vertexBufferStorage.vertexBuffers + vertexBufferHandle;
    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer->buffer, &stride, &offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw buffer with the shader
    deviceContext->Draw(vertexBuffer->verticesCount, 0);
}
