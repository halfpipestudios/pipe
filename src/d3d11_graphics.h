#ifndef _D3D11_GRAPHICS_H_
#define _D3D11_GRAPHICS_H_

#include <d3d11.h>
#include <d3dcompiler.h>

#include "graphics.h"


struct D3D11VertexBuffer {
    ID3D11Buffer *buffer;
    u32 verticesCount;
    u32 stride;
    u32 offset;
};

struct D3D11VertexBufferStorage {
    D3D11VertexBuffer vertexBuffers[256];
    i32 vertexBuffersCount;
};

struct D3D11Shader {
    ID3D11VertexShader *vertex;
    ID3D11PixelShader *fragment;
    ID3D11InputLayout *layout; 
};

struct D3D11ShaderStorage {
    D3D11Shader shaders[256];
    i32 shadersCount;
};

struct D3D11ConstBuffer {
    ID3D11Buffer *buffer;
    u32 index;
};

struct D3D11ConstBufferStorage {
    D3D11ConstBuffer constBuffers[256];
    i32 constBuffersCount;
};

struct D3D11TextureArray {
    ID3D11ShaderResourceView *srv;
    ID3D11Texture2D *gpuTextureArray;
    Texture    *cpuTextureArray;
    u32 mipLevels;
    u32 size;
};

struct D3D11TextureArrayStorage {
    D3D11TextureArray textureArrays[256];
    i32 textureArraysCount;
};

struct D3D11Graphics : public Graphics {
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11RasterizerState* wireFrameRasterizer;
    ID3D11RasterizerState* fillRasterizerCullBack;
    ID3D11RasterizerState* fillRasterizerCullFront;
    ID3D11RasterizerState* fillRasterizerCullNone;
    ID3D11DepthStencilState* depthStencilOn;
    ID3D11DepthStencilState* depthStencilOff;
    ID3D11BlendState* alphaBlendEnable;
    ID3D11BlendState* alphaBlendDisable;
    ID3D11SamplerState *samplerStateWrap;

    void Initialize() override;
    void Terminate() override;

    void SetRasterizerState(RasterizerState state) override;
    void SetDepthStencilState(bool value) override;
    void SetAlphaBlendState(bool value) override;

    void ClearColorBuffer(f32 r, f32 g, f32 b) override;
    void ClearDepthStencilBuffer() override;
    void Present(i32 vsync) override;

    Shader CreateShaderVertex(char *vertpath, char *fragpath) override;
    Shader CreateShaderVertexMap(char *vertpath, char *fragpath) override;
    void DestroyShader(Shader shaderHandle) override;

    virtual ConstBuffer CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName);
    virtual void DestroyConstBuffer(ConstBuffer constBufferHandle);
    virtual void UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData);

    void SetProjMatrix(Mat4 proj)   override; 
    void SetViewMatrix(Mat4 view)   override; 
    void SetWorldMatrix(Mat4 world) override;

    VertexBuffer CreateVertexBuffer(Vertex *vertices, u32 count) override;
    VertexBuffer CreateVertexBuffer(VertexMap *vertices, u32 count) override;
    void DestroyVertexBuffer(VertexBuffer vertexBufferHandle) override;
    void DrawVertexBuffer(VertexBuffer vertexBufferHandle, Shader shaderHandle) override;

    TextureArray CreateTextureArray(Texture *array, u32 textureCount) override;
    void DestroyTextureArray(TextureArray textureArrayHandle) override;
    void BindTextureArray(TextureArray textureArrayHandle) override;

    static D3D11ShaderStorage shadersStorage;
    static D3D11ConstBufferStorage constBufferStorage;
    static D3D11VertexBufferStorage vertexBufferStorage;
    static D3D11TextureArrayStorage textureArrayStorage;

};

#endif
