#include "d3d11_shadow_mapping.h"

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include <stdio.h>

void D3D11ShadowMappingBuilder::Initialize(ID3D11Device *device, u32 width_, u32 height_) {
    width = width_;
    height = height_;

    // save a viewport of the right size
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<f32>(width);
    viewport.Height = static_cast<f32>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // create the texture and the views
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    //texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT result = device->CreateTexture2D(&texDesc, 0, &depthMap);
    if(FAILED(result)) {
        printf("Error creating depth texture for building Shadow maps\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0;
    //dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    result = device->CreateDepthStencilView(depthMap, &dsvDesc, &depthMapDSV);
    if(FAILED(result)) {
        printf("Error creating depth stencil view for building Shadow maps\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // Create a readable texture
    D3D11_TEXTURE2D_DESC texDesc1;
    texDesc1.Width = width;
    texDesc1.Height = height;
    texDesc1.MipLevels = 1;
    texDesc1.ArraySize = 1;
    texDesc1.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc1.SampleDesc.Count = 1;
    texDesc1.SampleDesc.Quality = 0;
    texDesc1.Usage = D3D11_USAGE_STAGING;
    texDesc1.BindFlags = 0;
    texDesc1.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc1.MiscFlags = 0;
 
    if(FAILED(device->CreateTexture2D(&texDesc1, 0, &readableTexture))) {
        printf("Error creating readable texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    MemoryManager::Get()->EndTemporalMemory();

}

void D3D11ShadowMappingBuilder::Terminate() {
    if(depthMapDSV) depthMapDSV->Release();
    if(depthMap) depthMap->Release();
    if(readableTexture) readableTexture->Release();
}


void D3D11ShadowMappingBuilder::BindDepthBufferAsRenderTarget(ID3D11DeviceContext *dc) {
    dc->RSSetViewports(1, &viewport);

    ID3D11RenderTargetView *renderTragets[1] = { 0 };
    dc->OMSetRenderTargets(1, renderTragets, depthMapDSV);
    dc->ClearDepthStencilView(depthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11ShadowMappingBuilder::ReadDepthBuffer(ID3D11DeviceContext *dc, u8 **buffer, size_t &size) {
    dc->CopyResource(readableTexture, depthMap);
    
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    dc->Map(readableTexture, 0, D3D11_MAP_READ, 0, &bufferData);

    size = bufferData.RowPitch * SHADOW_MAP_RESOLUTION;
    memcpy(*buffer, bufferData.pData, size);

    dc->Unmap(readableTexture, 0);
}







