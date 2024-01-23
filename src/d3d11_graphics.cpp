#include "d3d11_graphics.h"

#include "platform_manager.h"
#include "memory_manager.h"

#include <stdio.h>


static i32 gMsaa = 4;

void D3D11Graphics::Initialize() {

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
    swapChainDesc.BufferCount = 2;
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
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOff);

    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOnWriteMaskZero);

    // TODO: create more blend state for particle system
    // additive blend state
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

    blendStateDesc.RenderTarget[0].BlendEnable = true;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDesc, &additiveBlending);

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

   // Create Sampler State
    D3D11_SAMPLER_DESC colorMapDesc = {};
    // D3D11_TEXTURE_ADDRESS_CLAMP; D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; 
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    if(FAILED(device->CreateSamplerState(&colorMapDesc, &samplerStatePoint))) {
        printf("Error: Failed Creating sampler state Point\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    if(FAILED(device->CreateSamplerState(&colorMapDesc, &samplerStateLinear))) {
        printf("Error: Failed Creating sampler state Linear\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
    deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
    deviceContext->RSSetState(fillRasterizerCullBack);
    deviceContext->PSSetSamplers(0, 1, &samplerStatePoint);
    deviceContext->GSSetSamplers(0, 1, &samplerStateLinear);

    cpuMatrices.proj = Mat4();
    cpuMatrices.view = Mat4();
    cpuMatrices.world = Mat4();
    
    cpuTGuiBuffer  = {};
    cpuGizmoBuffer = {};
    cpuIndexBuffer = {};

    CBParticle particlesData;

    gpuMatrices     = CreateConstBuffer(  (void *)&cpuMatrices,     sizeof(cpuMatrices),     0, nullptr);
    gpuAnimMatrices = CreateConstBuffer(  (void *)&cpuAnimMatrices, sizeof(cpuAnimMatrices), 1, nullptr);
    gpuParticleBuffer = CreateConstBuffer((void *)&particlesData,   sizeof(CBParticle),      2, nullptr);
    gpuGizmoBuffer  = CreateConstBuffer(  (void *)&cpuGizmoBuffer,  sizeof(cpuGizmoBuffer),  3, nullptr);
    gpuIndexBuffer  = CreateConstBuffer(  (void *)&cpuIndexBuffer,  sizeof(cpuIndexBuffer),  4, nullptr);
    gpuTGuiBuffer   = CreateConstBuffer(  (void *)&cpuTGuiBuffer,   sizeof(cpuTGuiBuffer),   5, nullptr);

    lineRenderer.Initialize(200, device);
    batchRenderer.Initialize(200, device);
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
    if(depthStencilOnWriteMaskZero) depthStencilOnWriteMaskZero->Release();
    if(alphaBlendEnable) alphaBlendEnable->Release();
    if(alphaBlendDisable) alphaBlendDisable->Release();
    if(additiveBlending) additiveBlending->Release();
    if(samplerStatePoint) samplerStatePoint->Release();
    if(samplerStateLinear) samplerStateLinear->Release();

    DestroyConstBuffer(gpuMatrices);
    DestroyConstBuffer(gpuAnimMatrices);
    DestroyConstBuffer(gpuTGuiBuffer);
    DestroyConstBuffer(gpuGizmoBuffer);
    DestroyConstBuffer(gpuIndexBuffer);
    DestroyConstBuffer(gpuParticleBuffer);

    lineRenderer.Terminate();
    batchRenderer.Terminate();
}

void D3D11Graphics::ResizeBuffers() {

    UINT msaaQuality4x;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, gMsaa, &msaaQuality4x);
    ASSERT(msaaQuality4x > 0);

    u32 width = PlatformManager::Get()->GetWindow()->GetWidth();
    u32 height = PlatformManager::Get()->GetWindow()->GetHeight();
    // first remove the invalid stuff
    if(renderTargetView) renderTargetView->Release();
    renderTargetView = nullptr;
    if(depthStencilView) depthStencilView->Release();
    depthStencilView = nullptr;

    swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
    if(backBuffer) backBuffer->Release();
    backBuffer = nullptr;

    // set up the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)width;
    viewport.Height = (f32)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = width;
    depthStencilTextureDesc.Height = height;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = gMsaa;
    depthStencilTextureDesc.SampleDesc.Quality = msaaQuality4x - 1;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
    HRESULT result = device->CreateTexture2D(&depthStencilTextureDesc, 0, &depthStencilTexture);
 
    result = device->CreateDepthStencilView(depthStencilTexture, 0, &depthStencilView);
    if (depthStencilTexture)
    {
        depthStencilTexture->Release();
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


void D3D11Graphics::SetSamplerState(SamplerState state) {
    switch(state) {
        case SAMPLER_STATE_POINT: {
            deviceContext->PSSetSamplers(0, 1, &samplerStatePoint);
        } break;
        case SAMPLER_STATE_LINEAR: {
            deviceContext->PSSetSamplers(0, 1, &samplerStateLinear);
        } break;
    }
}

void  D3D11Graphics::SetDepthStencilState(bool value) {
    if(value)
        deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
    else
        deviceContext->OMSetDepthStencilState(depthStencilOff, 1);
}

void  D3D11Graphics::SetDepthStencilWriteZeroState(bool value) {
    if(value)
        deviceContext->OMSetDepthStencilState(depthStencilOnWriteMaskZero, 1);
    else
        deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
}

void D3D11Graphics::SetAlphaBlendState(bool value) {
    if(value)
        deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
    else
        deviceContext->OMSetBlendState(alphaBlendDisable, 0, 0xffffffff);
}

void D3D11Graphics::SetAdditiveBlendState(bool value) {
    if(value)
        deviceContext->OMSetBlendState(additiveBlending, 0, 0xffffffff);
    else
        deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
}



void D3D11Graphics::ClearColorBuffer(FrameBuffer frameBufferHandle, f32 r, f32 g, f32 b) {
    float clearColor[] = { r, g, b, 1.0f };
    if(frameBufferHandle == nullptr) {
        deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
    } else {
        D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
        deviceContext->ClearRenderTargetView(frameBuffer->renderTargetView, clearColor);
    }
}

void D3D11Graphics::ClearDepthStencilBuffer(FrameBuffer frameBufferHandle) {
    if(frameBufferHandle == nullptr) {
        deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
    } else {
        D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
        deviceContext->ClearDepthStencilView(frameBuffer->depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void D3D11Graphics::Present(i32 vsync) {
    swapChain->Present(vsync, 0);
}

struct InputLayoutDesc {
    D3D11_INPUT_ELEMENT_DESC desc[5];
    i32 count = 0;
};

static InputLayoutDesc inputLayoutDescArray[5] = {
    {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
             0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        }, 3
    },
    {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
             0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32_UINT,
             0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
        }, 4
    },
    {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
             0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_SINT,
             0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
             0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
        }, 5
    },
    {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,
             0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
             0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
        }, 3
    },
    {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,
             0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,
             0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT,
             0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 3, DXGI_FORMAT_R32_UINT,
             0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
        }, 5
    }

};

static VShader CreateVShader_(ID3D11Device *device, ObjectAllocator<D3D11VertexShader>* shadersStorage, char *vertpath) {
    D3D11VertexShader shader = {}; 

    MemoryManager::Get()->BeginTemporalMemory();
    File vertfile = PlatformManager::Get()->ReadFileToTemporalMemory(vertpath);

    ID3DBlob *vertexShaderCompiled = 0;
    
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

    // create the vertex and fragment shader
    result = device->CreateVertexShader(
            vertexShaderCompiled->GetBufferPointer(),
            vertexShaderCompiled->GetBufferSize(), 0,
            &shader.vertex);

    MemoryManager::Get()->EndTemporalMemory();

    bool inputLayoutCreated = false;
    for(i32 i = 0; i < ARRAY_LENGTH(inputLayoutDescArray); i++) {
        InputLayoutDesc *inputLayoutDesc = inputLayoutDescArray + i;

        // create input layout
        HRESULT layoutResult = device->CreateInputLayout(
                inputLayoutDesc->desc,
                inputLayoutDesc->count,
                vertexShaderCompiled->GetBufferPointer(),
                vertexShaderCompiled->GetBufferSize(),
                &shader.layout);
        if(!FAILED(layoutResult)) {
            inputLayoutCreated = true;
            break;
        } else {
            if(shader.layout) shader.layout->Release();
        }
    }

    ASSERT(inputLayoutCreated == true);

    if(vertexShaderCompiled) vertexShaderCompiled->Release();

    D3D11VertexShader *shaderHandle = shadersStorage->Alloc();
    *shaderHandle = shader;
    return (VShader)shaderHandle;
}

VShader D3D11Graphics::CreateVShader(char *vertpath) {
    return CreateVShader_(device, &vertShadersStorage, vertpath);
}

void D3D11Graphics::DestroyVShader(VShader shaderHandle) {
    D3D11VertexShader *shader = (D3D11VertexShader *)shaderHandle;
    if(shader->vertex) shader->vertex->Release();
    if(shader->layout) shader->layout->Release();
    vertShadersStorage.Free(shader);
}

void D3D11Graphics::BindVShader(VShader shaderHandle) {
    D3D11VertexShader *shader = (D3D11VertexShader *)shaderHandle;
    deviceContext->VSSetShader(shader->vertex, 0, 0);
    deviceContext->IASetInputLayout(shader->layout);
}

static FShader CreateFShader_(ID3D11Device *device, ObjectAllocator<D3D11FragmentShader>* shadersStorage,
                             char *fragpath) {
    D3D11FragmentShader shader = {}; 

    MemoryManager::Get()->BeginTemporalMemory();
    File fragfile = PlatformManager::Get()->ReadFileToTemporalMemory(fragpath);

    ID3DBlob *fragmentShaderCompiled = 0;
    
    HRESULT result = 0;

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

    result = device->CreatePixelShader(
            fragmentShaderCompiled->GetBufferPointer(),
            fragmentShaderCompiled->GetBufferSize(), 0,
            &shader.fragment);

    MemoryManager::Get()->EndTemporalMemory();

    if(fragmentShaderCompiled) fragmentShaderCompiled->Release();

    D3D11FragmentShader *shaderHandle = shadersStorage->Alloc();
    *shaderHandle = shader;
    return (FShader)shaderHandle;
}


FShader D3D11Graphics::CreateFShader(char *fragpath) {
    return CreateFShader_(device, &fragShadersStorage, fragpath);
}

void D3D11Graphics::DestroyFShader(FShader shaderHandle) {
    D3D11FragmentShader *shader = (D3D11FragmentShader *)shaderHandle;
    if(shader->fragment) shader->fragment->Release();
    fragShadersStorage.Free(shader);
}

void D3D11Graphics::BindFShader(FShader shaderHandle) {
    D3D11FragmentShader *shader = (D3D11FragmentShader *)shaderHandle;
    deviceContext->PSSetShader(shader->fragment, 0, 0);
}

GeometryShader D3D11Graphics::CreateGeometryShader(char *filepath) {
    D3D11GeometryShader shader;

    MemoryManager::Get()->BeginTemporalMemory();
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(filepath);

    ID3DBlob *shaderCompiled = 0;
    
    HRESULT result = 0;
    ID3DBlob *errorShader = 0;
    result = D3DCompile(file.data, file.size,
                        0, 0, 0, "GS", "gs_4_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &shaderCompiled,
                        &errorShader);
    if(errorShader != 0) {
        char *errorString = (char *)errorShader->GetBufferPointer();
        printf("error compiling geometry shader (%s): %s", filepath, errorString);
        errorShader->Release();
        ASSERT(!"INVALID_CODE_PATH");
    }

    result = device->CreateGeometryShader(
        shaderCompiled->GetBufferPointer(),
        shaderCompiled->GetBufferSize(),
        nullptr,
        &shader.geometry);

    if(FAILED(result)) {
        printf("Error: Failed Creating Geometry Shader\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    MemoryManager::Get()->EndTemporalMemory();

    D3D11GeometryShader *shaderHandle = geometryShadersStorage.Alloc();
    *shaderHandle = shader;
    return (GeometryShader)shaderHandle;
}

GeometryShader  D3D11Graphics::CreateGeometryShaderWithStreamOutput(char *filepath) {

    D3D11GeometryShader shader;

    MemoryManager::Get()->BeginTemporalMemory();
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(filepath);

    ID3DBlob *shaderCompiled = 0;
    
    HRESULT result = 0;
    ID3DBlob *errorShader = 0;
    result = D3DCompile(file.data, file.size,
                        0, 0, 0, "GS", "gs_4_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &shaderCompiled,
                        &errorShader);
    if(errorShader != 0) {
        char *errorString = (char *)errorShader->GetBufferPointer();
        printf("error compiling geometry shader (%s): %s", filepath, errorString);
        errorShader->Release();
        ASSERT(!"INVALID_CODE_PATH");
    }

    D3D11_SO_DECLARATION_ENTRY pDecl[] = {
        { 0, "POSITION",   0, 0, 3, 0 }, 
        { 0, "TEXCOORD",   0, 0, 3, 0 }, 
        { 0, "TEXCOORD",   1, 0, 2, 0 }, 
        { 0, "TEXCOORD",   2, 0, 1, 0 }, 
        { 0, "TEXCOORD",   3, 0, 1, 0 }
    };

    result = device->CreateGeometryShaderWithStreamOutput(
        shaderCompiled->GetBufferPointer(),
        shaderCompiled->GetBufferSize(),
        pDecl, ARRAY_LENGTH(pDecl),
        nullptr, 0, 0, nullptr,
        &shader.geometry);
    
    if(FAILED(result)) {
        printf("Error: Failed Creating Geometry Shader\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    MemoryManager::Get()->EndTemporalMemory();

    D3D11GeometryShader *shaderHandle = geometryShadersStorage.Alloc();
    *shaderHandle = shader;
    return (GeometryShader)shaderHandle;
}

void D3D11Graphics::DestroyGeometryShader(GeometryShader geometryShaderHandle) {
    D3D11GeometryShader *shader = (D3D11GeometryShader *)geometryShaderHandle;
    if(shader->geometry) shader->geometry->Release();
    geometryShadersStorage.Free(shader);
}

void D3D11Graphics::BindGeometryShader(GeometryShader geometryShaderHandle) {
    D3D11GeometryShader *shader = (D3D11GeometryShader *)geometryShaderHandle;
    deviceContext->GSSetShader(shader->geometry, 0, 0);
}

void D3D11Graphics::DisablePixelShader() {
    deviceContext->PSSetShader(nullptr, 0, 0);
}

void D3D11Graphics::DisableVertexShader() {
    deviceContext->VSSetShader(nullptr, 0, 0);
}

void D3D11Graphics::DisableGeometryShader() {
    deviceContext->GSSetShader(nullptr, 0, 0);
}

ConstBuffer D3D11Graphics::CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName) {
    D3D11ConstBuffer constBuffer = {};

    D3D11_BUFFER_DESC constBufferDesc;
    ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufferDesc.ByteWidth = bufferSize;
    constBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT result = device->CreateBuffer(&constBufferDesc, 0, &constBuffer.buffer);
    if(FAILED(result)) {
        printf("error creating const buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    deviceContext->UpdateSubresource(constBuffer.buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(index, 1, &constBuffer.buffer);
    deviceContext->PSSetConstantBuffers(index, 1, &constBuffer.buffer);
    deviceContext->GSSetConstantBuffers(index, 1, &constBuffer.buffer);
    constBuffer.index = index;

    D3D11ConstBuffer *constBufferHandle = constBufferStorage.Alloc();
    *constBufferHandle = constBuffer;
    return (ConstBuffer)constBufferHandle;
}

void D3D11Graphics::DestroyConstBuffer(ConstBuffer constBufferHandle) {
    D3D11ConstBuffer *constBuffer = (D3D11ConstBuffer *)constBufferHandle;
    if(constBuffer->buffer) constBuffer->buffer->Release();
    constBuffer->index = -1;
    constBufferStorage.Free(constBuffer);
}

void D3D11Graphics::UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData) {
    D3D11ConstBuffer *constBuffer = (D3D11ConstBuffer *)constBufferHandle;
    deviceContext->UpdateSubresource(constBuffer->buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
    deviceContext->PSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
    deviceContext->GSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
}


void D3D11Graphics::BindConstBuffer(ConstBuffer constBufferHandle, u32 slot) {
    D3D11ConstBuffer *constBuffer = (D3D11ConstBuffer *)constBufferHandle;
    constBuffer->index = slot;
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

void D3D11Graphics::SetAnimMatrices(Mat4 *finalTransformMatrices, u32 count) {
    memcpy(cpuAnimMatrices.boneMatrix, finalTransformMatrices, MIN(count, 100)*sizeof(Mat4));
    UpdateConstBuffer(gpuAnimMatrices, (void *)&cpuAnimMatrices);
}

VertexBuffer D3D11Graphics::CreateVertexBuffer(void *vertices, u32 count, size_t stride) {

    D3D11VertexBuffer vertexBuffer = {};
    vertexBuffer.verticesCount = count;

    vertexBuffer.stride = stride;
    vertexBuffer.offset = 0;

    // create gpu buffer
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = stride * count;
    resourceData.pSysMem = vertices;

    HRESULT result = device->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer.buffer);
    if(FAILED(result)) {
        printf("error loading Vertex buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    D3D11VertexBuffer *vertexBufferHandle = vertexBufferStorage.Alloc();
    *vertexBufferHandle = vertexBuffer;
    return (VertexBuffer)vertexBufferHandle;
}

void D3D11Graphics::DestroyVertexBuffer(VertexBuffer vertexBufferHandle) {
    D3D11VertexBuffer *vertexBuffer = (D3D11VertexBuffer *)vertexBufferHandle;
    if(vertexBuffer->buffer) vertexBuffer->buffer->Release();
    vertexBuffer->verticesCount = 0;
    vertexBuffer->stride = 0;
    vertexBuffer->offset = 0;
    vertexBufferStorage.Free(vertexBuffer);
}

void D3D11Graphics::DrawVertexBuffer(VertexBuffer vertexBufferHandle, VShader vshaderHandle, FShader fshaderHandle) {
    // set shader
    D3D11VertexShader *vShader = (D3D11VertexShader *)vshaderHandle;
    D3D11FragmentShader *fShader = (D3D11FragmentShader *)fshaderHandle;

    deviceContext->VSSetShader(vShader->vertex, 0, 0);
    deviceContext->PSSetShader(fShader->fragment, 0, 0);
    deviceContext->IASetInputLayout(vShader->layout);

    // set buffer
    D3D11VertexBuffer *vertexBuffer = (D3D11VertexBuffer *)vertexBufferHandle;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer->buffer, &vertexBuffer->stride, &vertexBuffer->offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw buffer with the shader
    deviceContext->Draw(vertexBuffer->verticesCount, 0);
}

IndexBuffer D3D11Graphics::CreateIndexBuffer(u32 *indices, u32 count) {
    D3D11IndexBuffer indexBuffer = {};
    indexBuffer.indexCount = count;
    indexBuffer.format = DXGI_FORMAT_R32_UINT;

    // create gpu buffer
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    D3D11_BUFFER_DESC indexDesc;
    ZeroMemory(&indexDesc, sizeof(indexDesc));
    indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.ByteWidth = sizeof(u32) * count;
    resourceData.pSysMem = indices;

    HRESULT result = device->CreateBuffer(&indexDesc, &resourceData, &indexBuffer.buffer);
    if(FAILED(result)) {
        printf("error loading Index buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    D3D11IndexBuffer *indexBufferHandle = indexBufferStorage.Alloc();
    *indexBufferHandle = indexBuffer;
    return (IndexBuffer)indexBufferHandle;
}

void D3D11Graphics::DestroyIndexBuffer(IndexBuffer indexBuffer) {
    D3D11IndexBuffer *indexBufferHandle = (D3D11IndexBuffer *)indexBuffer;
    if(indexBufferHandle->buffer) indexBufferHandle->buffer->Release();
    indexBufferStorage.Free(indexBufferHandle);
}

void D3D11Graphics::DrawIndexBuffer(IndexBuffer indexBuffer, VertexBuffer vertexBuffer, VShader vshaderHandle, FShader fshaderHandle) {
    
    // NOTE: set shader
    D3D11VertexShader *vShader = (D3D11VertexShader *)vshaderHandle;
    D3D11FragmentShader *fShader = (D3D11FragmentShader *)fshaderHandle;

    deviceContext->VSSetShader(vShader->vertex, 0, 0);
    deviceContext->PSSetShader(fShader->fragment, 0, 0);
    deviceContext->IASetInputLayout(vShader->layout);

    // NOTE: set buffer
    D3D11VertexBuffer *vertexBufferHandle = (D3D11VertexBuffer *)vertexBuffer;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBufferHandle->buffer, &vertexBufferHandle->stride, &vertexBufferHandle->offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // NOTE: set indices
    D3D11IndexBuffer *indexBufferHandle = (D3D11IndexBuffer *)indexBuffer;
    deviceContext->IASetIndexBuffer(indexBufferHandle->buffer, indexBufferHandle->format, 0);

    // NOTE: Draw
    deviceContext->DrawIndexed(indexBufferHandle->indexCount, 0, 0);

}

TextureBuffer D3D11Graphics::CreateTextureBuffer(Texture *array, u32 textureCount) {

    D3D11TextureArray textureArray = {};
    textureArray.size = textureCount;
    textureArray.cpuTextureArray = array;
    textureArray.mipLevels = 8;

    // Create the GPU stuff
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = array[0].w;
    texDesc.Height = array[0].h;
    texDesc.MipLevels = textureArray.mipLevels;
    texDesc.ArraySize = textureArray.size;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &textureArray.gpuTextureArray)))
    {
        printf("Error creating Texture Array GPU Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    for(i32 i = 0; i < textureArray.size; ++i)
    {
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = textureArray.cpuTextureArray[i].pixels;
        data.SysMemPitch  = textureArray.cpuTextureArray[i].w*sizeof(u32);
        data.SysMemSlicePitch = 0;
        deviceContext->UpdateSubresource(textureArray.gpuTextureArray,
                D3D11CalcSubresource(0, i, textureArray.mipLevels),
                0, data.pSysMem, data.SysMemPitch, 0);
    }

    // create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MipLevels = textureArray.mipLevels;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = textureArray.size;
    if (FAILED(device->CreateShaderResourceView(textureArray.gpuTextureArray, &srvDesc, &textureArray.srv)))
    {
        printf("Error creating Texture Array srv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    deviceContext->GenerateMips(textureArray.srv);

    D3D11TextureArray *textureArrayHandle = textureArrayStorage.Alloc();
    *textureArrayHandle = textureArray;
    return (TextureBuffer)textureArrayHandle;
}

void D3D11Graphics::DestroyTextureBuffer(TextureBuffer textureBufferHandle) {
    D3D11TextureArray *textureArray = (D3D11TextureArray *)textureBufferHandle;

    if(textureArray->srv) { 
        textureArray->srv->Release(); 
        textureArray->srv = nullptr;
    }
    if(textureArray->gpuTextureArray) { 
        textureArray->gpuTextureArray->Release();
        textureArray->gpuTextureArray = nullptr; 
    }
    textureArray->size = 0;
    textureArray->mipLevels = 0;

    textureArrayStorage.Free(textureArray);
    
}

void D3D11Graphics::BindTextureBuffer(TextureBuffer textureBufferHandle) {
    D3D11TextureArray *textureArray = (D3D11TextureArray *)textureBufferHandle;
    deviceContext->PSSetShaderResources(0, 1, &textureArray->srv);
}

void D3D11Graphics::FrameBufferMap(FrameBuffer frameBufferHandle, u32 *w, u32 *h, u32 *sizeInBytes, u8 **buffer) {
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    
    D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
    
    deviceContext->Map(frameBuffer->texture, 0, D3D11_MAP_READ, 0, &bufferData);
    *w = (u32)frameBuffer->w;
    *h = (u32)frameBuffer->h;
    *buffer = (u8 *)bufferData.pData;
    *sizeInBytes = bufferData.RowPitch * (u32)frameBuffer->h;
}

void D3D11Graphics::FrameBufferUnmap(FrameBuffer frameBufferHandle) {
    D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
    deviceContext->Unmap(frameBuffer->texture, 0);
}

FrameBuffer D3D11Graphics::CreateFrameBuffer(u32 x, u32 y, u32 width, u32 height) {
    
    D3D11FrameBuffer frameBuffer = {};
    
    frameBuffer.x = (f32)x;
    frameBuffer.y = (f32)y;
    frameBuffer.w = (f32)width;
    frameBuffer.h = (f32)height;
    frameBuffer.format = DXGI_FORMAT_R8G8B8A8_UNORM;
    
    // create texture 2d
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = frameBuffer.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &frameBuffer.texture))) {
        printf("Error creating FrameBuffer Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = frameBuffer.format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    if(FAILED(device->CreateRenderTargetView(frameBuffer.texture, &rtvDesc, &frameBuffer.renderTargetView))) {
        printf("Error creating FrameBuffer rtv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = frameBuffer.format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(device->CreateShaderResourceView(frameBuffer.texture, &srvDesc, &frameBuffer.shaderResourceView))) {
        printf("Error creating FrameBuffer srv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width  = width;
    depthStencilTextureDesc.Height = height;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
 
    // create the depth stencil view
    if(FAILED(device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture))) {
        printf("Error creating FrameBuffer depthStencilTexture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if(FAILED(device->CreateDepthStencilView(depthStencilTexture, &descDSV, &frameBuffer.depthStencilView))) {
        printf("Error creating FrameBuffer dsv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    if (depthStencilTexture) {
        depthStencilTexture->Release();
    }

    frameBuffer.textureBuffer = {};
    
    frameBuffer.textureBuffer.srv = frameBuffer.shaderResourceView;
    frameBuffer.textureBuffer.gpuTextureArray = frameBuffer.texture;
    frameBuffer.textureBuffer.mipLevels = 1;
    frameBuffer.textureBuffer.size = 1;

    D3D11FrameBuffer *frameBufferHandle = frameBufferStorage.Alloc();
    *frameBufferHandle = frameBuffer;

    return (FrameBuffer)frameBufferHandle;
}

void D3D11Graphics::CopyFrameBuffer(FrameBuffer desHandle, FrameBuffer srcHandle) {
    D3D11FrameBuffer *des = (D3D11FrameBuffer *)desHandle;
    D3D11FrameBuffer *src = (D3D11FrameBuffer *)srcHandle;
    deviceContext->CopyResource(des->texture, src->texture);
}

FrameBuffer D3D11Graphics::CreateReadFrameBuffer(u32 x, u32 y, u32 width, u32 height) {
    
    D3D11FrameBuffer frameBuffer = {};
    
    frameBuffer.x = (f32)x;
    frameBuffer.y = (f32)y;
    frameBuffer.w = (f32)width;
    frameBuffer.h = (f32)height;
    frameBuffer.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    
    // create texture 2d
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = frameBuffer.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc.MiscFlags = 0;
    
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &frameBuffer.texture))) {
        printf("Error creating FrameBuffer Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    frameBuffer.textureBuffer = {};
    frameBuffer.textureBuffer.gpuTextureArray = frameBuffer.texture;
    frameBuffer.textureBuffer.mipLevels = 1;
    frameBuffer.textureBuffer.size = 1;

    D3D11FrameBuffer *frameBufferHandle = frameBufferStorage.Alloc();
    *frameBufferHandle = frameBuffer;

    return (FrameBuffer)frameBufferHandle;
}

FrameBuffer D3D11Graphics::CreateWriteFrameBuffer(u32 x, u32 y, u32 width, u32 height) {
    
    D3D11FrameBuffer frameBuffer = {};
    
    frameBuffer.x = (f32)x;
    frameBuffer.y = (f32)y;
    frameBuffer.w = (f32)width;
    frameBuffer.h = (f32)height;
    frameBuffer.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    
    // create texture 2d
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = frameBuffer.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &frameBuffer.texture))) {
        printf("Error creating FrameBuffer Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = frameBuffer.format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    if(FAILED(device->CreateRenderTargetView(frameBuffer.texture, &rtvDesc, &frameBuffer.renderTargetView))) {
        printf("Error creating FrameBuffer rtv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width  = width;
    depthStencilTextureDesc.Height = height;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
 
    // create the depth stencil view
    if(FAILED(device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture))) {
        printf("Error creating FrameBuffer depthStencilTexture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if(FAILED(device->CreateDepthStencilView(depthStencilTexture, &descDSV, &frameBuffer.depthStencilView))) {
        printf("Error creating FrameBuffer dsv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    if (depthStencilTexture) {
        depthStencilTexture->Release();
    }

    frameBuffer.textureBuffer = {};
    
    frameBuffer.textureBuffer.srv = frameBuffer.shaderResourceView;
    frameBuffer.textureBuffer.gpuTextureArray = frameBuffer.texture;
    frameBuffer.textureBuffer.mipLevels = 1;
    frameBuffer.textureBuffer.size = 1;

    D3D11FrameBuffer *frameBufferHandle = frameBufferStorage.Alloc();
    *frameBufferHandle = frameBuffer;

    return (FrameBuffer)frameBufferHandle;
}

void D3D11Graphics::DestroyFrameBuffer(FrameBuffer frameBufferHandle) {

    D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
    
    if(frameBuffer->texture)            { frameBuffer->texture->Release();            frameBuffer->texture            = 0; }
    if(frameBuffer->renderTargetView)   { frameBuffer->renderTargetView->Release();   frameBuffer->renderTargetView   = 0; }
    if(frameBuffer->shaderResourceView) { frameBuffer->shaderResourceView->Release(); frameBuffer->shaderResourceView = 0; }
    if(frameBuffer->depthStencilView)   { frameBuffer->depthStencilView->Release();   frameBuffer->depthStencilView   = 0; }
    
    frameBufferStorage.Free(frameBuffer);
}

void D3D11Graphics::BindFrameBuffer(FrameBuffer frameBufferHandle) {
    if(frameBufferHandle == nullptr) {
        deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    } else {
        D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
        deviceContext->OMSetRenderTargets(1, &frameBuffer->renderTargetView, frameBuffer->depthStencilView);
    }
}

TextureBuffer D3D11Graphics::FrameBufferGetTexture(FrameBuffer frameBufferHandle) {
    D3D11FrameBuffer *frameBuffer = (D3D11FrameBuffer *)frameBufferHandle;
    return (TextureBuffer *)&frameBuffer->textureBuffer; 
}


void D3D11Graphics::FlushFrameBuffer(FrameBuffer frameBufferHandle) {
    lineRenderer.Render(deviceContext);
}

ParticleSystem D3D11Graphics::CreateParticleSystem(u32 maxParticle,
                                                   VShader soVShader, FShader soFShader, GeometryShader soGShader,
                                                   VShader dwVShader, FShader dwFShader, GeometryShader dwGShader,
                                                   Handle texture) {

    D3D11ParticleSystem particleSystem;
    D3D11ParticleSystem *particleSystemHandle = particleSystemStoraget.Alloc();
    *particleSystemHandle = particleSystem;

    particleSystemHandle->Initialize(device, maxParticle,
                                     soVShader, soFShader, soGShader,
                                     dwVShader, dwFShader, dwGShader,
                                     gpuParticleBuffer, texture);

    return (ParticleSystem)particleSystemHandle;

}

void D3D11Graphics::DestroyParticleSystem(ParticleSystem particleSystemHandle) {

    D3D11ParticleSystem *particleSystem = (D3D11ParticleSystem *)particleSystemHandle;
    particleSystem->Terminate();
    particleSystemStoraget.Free(particleSystem);

}

void D3D11Graphics::ResetParticleSystem(ParticleSystem particleSystemHandle) {
    D3D11ParticleSystem *particleSystem = (D3D11ParticleSystem *)particleSystemHandle;
    particleSystem->Reset();
}

void D3D11Graphics::UpdateParticleSystem(ParticleSystem particleSystemHandle, Vec3 startPos, Vec3 targetPos, Vec3 cameraPos, f32 gameTime, f32 dt) {
    D3D11ParticleSystem *particleSystem = (D3D11ParticleSystem *)particleSystemHandle;
    particleSystem->Update(startPos, targetPos, cameraPos, gameTime, dt);
}

void D3D11Graphics::RenderParticleSystem(ParticleSystem particleSystemHandle) {
    D3D11ParticleSystem *particleSystem = (D3D11ParticleSystem *)particleSystemHandle;
    particleSystem->Draw(device, deviceContext);
}

void D3D11Graphics::SetViewport(u32 x, u32 y, u32 w, u32 h) {
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = (f32)x;
    viewport.TopLeftY = (f32)y;
    viewport.Width    = (f32)w;
    viewport.Height   = (f32)h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}

void D3D11Graphics::DrawLine(Vec3 a, Vec3 b, u32 color) {
    lineRenderer.DrawLine(a, b, color, deviceContext);
}

void D3D11Graphics::Draw2DBatch(D3D112DVertex *vertices, u32 vertexCount, u32 *indices, u32 indexCount) { 
    batchRenderer.AddBatchVertex(vertices, vertexCount, indices, indexCount, deviceContext);
    batchRenderer.Render(deviceContext);
}


// LINE RENDERER 
//------------------------------------------------------------------------------------------------------
void D3D11LineRenderer::Initialize(size_t bufferSize_, ID3D11Device *device) {
    bufferUsed = 0;
    bufferSize = bufferSize_;
    bufferSizeInBytes = bufferSize * sizeof(D3D11VertexLine);
    shader = CreateD3D11Shader(device,
                               "./data/shaders/lineVert.hlsl",
                               "./data/shaders/lineFrag.hlsl");
    // Create the GPU buffer
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexDesc.ByteWidth = bufferSizeInBytes;
    HRESULT result = device->CreateBuffer(&vertexDesc, NULL, &gpuLineBuffer);
    if(FAILED(result)) {
        printf("Error create line renderer GPU buffer.\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    // Create the CPU buffer
    cpuLineBuffer = (D3D11VertexLine *)MemoryManager::Get()->AllocStaticMemory(bufferSizeInBytes, 1);
    memset(cpuLineBuffer, 0, bufferSizeInBytes);
}

void D3D11LineRenderer::Terminate() {
    if(gpuLineBuffer) gpuLineBuffer->Release();
    DestroyD3D11Shader(&shader);
}

void D3D11LineRenderer::Render(ID3D11DeviceContext *deviceContext) {
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(gpuLineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, cpuLineBuffer, sizeof(D3D11VertexLine)*bufferUsed);
    deviceContext->Unmap(gpuLineBuffer, 0);

    u32 stride = sizeof(D3D11VertexLine);
    u32 offset = 0;
    deviceContext->IASetInputLayout(shader.layout);
    deviceContext->VSSetShader(shader.vertex, 0, 0);
    deviceContext->PSSetShader(shader.fragment, 0, 0);
    deviceContext->IASetVertexBuffers(0, 1, &gpuLineBuffer, &stride, &offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    deviceContext->Draw(bufferUsed, 0);
    bufferUsed = 0;
}

Vec4 D3D11LineRenderer::Vec4Color(u32 color) {
    f32 a = (f32)((color >> 24) & 0xFF) / 255.0f;
    f32 r = (f32)((color >> 16) & 0xFF) / 255.0f;
    f32 g = (f32)((color >>  8) & 0xFF) / 255.0f;
    f32 b = (f32)((color >>  0) & 0xFF) / 255.0f;
    return {r, g, b, a};
}

void D3D11LineRenderer::DrawLine(Vec3 a, Vec3 b, u32 color_, ID3D11DeviceContext *deviceContext) {

    Vec4 color = Vec4Color(color_);

    D3D11VertexLine line[2] = {};
    line[0].pos = a;
    line[0].col = color;
    line[1].pos = b;
    line[1].col = color;

    if(bufferUsed + 2 >= bufferSize) {
        Render(deviceContext);
    }

    AddLine(line);
}

void D3D11LineRenderer::AddLine(D3D11VertexLine *line) {
    ASSERT(bufferUsed + 2 < bufferSize);
    D3D11VertexLine *vertex = cpuLineBuffer + bufferUsed;
    memcpy(vertex, line, sizeof(D3D11VertexLine)*2);
    bufferUsed += 2;
}

D3D11Shader D3D11LineRenderer::CreateD3D11Shader(ID3D11Device *device, char *vertpath, char *fragpath)
{
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
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

    return shader;
}

void D3D11LineRenderer::DestroyD3D11Shader(D3D11Shader *shader) {
    if(shader->vertex) shader->vertex->Release();
    if(shader->fragment) shader->fragment->Release();
    if(shader->layout) shader->layout->Release();
}

void D3D11BatchRenderer::Initialize(size_t bufferSize, ID3D11Device *device) {
    // NOTE: Initialize vertex buffer -------------------------------------
    
    HRESULT result = 0;

    cpuQuadBuffer.used = 0;
    cpuQuadBuffer.size = bufferSize;
    
    // Create the GPU buffer
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexDesc.ByteWidth = cpuQuadBuffer.size * sizeof(D3D11Quad);
    result = device->CreateBuffer(&vertexDesc, NULL, &gpuVertexBuffer);
    if(FAILED(result)) {
        printf("Error: cannot create Batch GPU buffer.\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    // Create the CPU buffer
    cpuQuadBuffer.quads = (D3D11Quad *)MemoryManager::Get()->AllocStaticMemory(cpuQuadBuffer.size * sizeof(D3D11Quad), 1);
    memset(cpuQuadBuffer.quads, 0, cpuQuadBuffer.size * sizeof(D3D11Quad));

    // Create the GPU buffer
    D3D11_BUFFER_DESC indexDesc;
    ZeroMemory(&indexDesc, sizeof(indexDesc));
    indexDesc.Usage = D3D11_USAGE_DYNAMIC;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    indexDesc.ByteWidth = cpuQuadBuffer.size * sizeof(u32) * 6;
    result = device->CreateBuffer(&indexDesc, NULL, &gpuIndexBuffer);
    if(FAILED(result)) {
        printf("Error: cannot create Batch GPU buffer.\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    cpuQuadBuffer.indices = (u32 *)MemoryManager::Get()->AllocStaticMemory(cpuQuadBuffer.size * sizeof(u32) * 6, 1);
    memset(cpuQuadBuffer.indices, 0, cpuQuadBuffer.size * sizeof(u32) * 6);

}

void D3D11BatchRenderer::Terminate() {
    if(gpuVertexBuffer) gpuVertexBuffer->Release();
    if(gpuIndexBuffer) gpuIndexBuffer->Release();
}

void D3D11BatchRenderer::Render(ID3D11DeviceContext *deviceContext) {

    D3D11_MAPPED_SUBRESOURCE bufferData;
    
    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(gpuVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, cpuQuadBuffer.quads, cpuQuadBuffer.used * sizeof(D3D11Quad));
    deviceContext->Unmap(gpuVertexBuffer, 0);


    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(gpuIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, cpuQuadBuffer.indices, cpuQuadBuffer.used * sizeof(u32) * 6);
    deviceContext->Unmap(gpuIndexBuffer, 0);

    u32 stride = sizeof(D3D112DVertex);
    u32 offset = 0;
    
    deviceContext->IASetVertexBuffers(0, 1, &gpuVertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(gpuIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->DrawIndexed(cpuQuadBuffer.used * 6, 0, 0);
    cpuQuadBuffer.used = 0;
}

void D3D11BatchRenderer::AddBatchVertex(D3D112DVertex *vertices, u32 vertexCount, u32 *indices, u32 indexCount, ID3D11DeviceContext *deviceContext) {
    
    u32 quadCount = (vertexCount / 4);

    for(u32 quadIndex = 0; quadIndex < quadCount; ++quadIndex) {
        
        memcpy(cpuQuadBuffer.quads + cpuQuadBuffer.used, vertices + quadIndex * 4, sizeof(D3D11Quad));
        
        // NOTE: Fix by manuto uzumaki
        if(quadIndex < cpuQuadBuffer.size) {
            memcpy(cpuQuadBuffer.indices + cpuQuadBuffer.used * 6, indices + quadIndex * 6, sizeof(u32) * 6);
        }
        
        ++cpuQuadBuffer.used;

        if(cpuQuadBuffer.used == cpuQuadBuffer.size) {
            Render(deviceContext);
        }

    }
}
