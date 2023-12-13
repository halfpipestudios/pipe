#ifndef _D3D11_GRAPHICS_H_
#define _D3D11_GRAPHICS_H_

#include <d3d11.h>
#include <d3dcompiler.h>

#include "graphics.h"
#include "allocators.h"

struct D3D11VertexBuffer {
    ID3D11Buffer *buffer;
    u32 verticesCount;
    u32 stride;
    u32 offset;
};

struct D3D11IndexBuffer {
    ID3D11Buffer *buffer;
    u32 indexCount;
    DXGI_FORMAT format;
};

struct D3D11TextureArray {
    ID3D11ShaderResourceView *srv;
    ID3D11Texture2D *gpuTextureArray;
    Texture    *cpuTextureArray;
    u32 mipLevels;
    u32 size;
};

struct D3D11FrameBuffer {
    
    f32 x, y, w, h;
    DXGI_FORMAT format;
    ID3D11Texture2D *texture;
    ID3D11RenderTargetView   *renderTargetView;
    ID3D11ShaderResourceView *shaderResourceView;
    ID3D11DepthStencilView   *depthStencilView; 
    
    D3D11TextureArray textureBuffer;
};

struct D3D11Shader {
    ID3D11VertexShader *vertex;
    ID3D11PixelShader *fragment;
    ID3D11InputLayout *layout; 
};

struct D3D11ConstBuffer {
    ID3D11Buffer *buffer;
    u32 index;
};

struct D3D11VertexLine {
    Vec3 pos;
    Vec4 col;
};

struct D3D11Quad {
    D3D112DVertex vertices[4];
};

struct D3D11QuadBatch {
    D3D11Quad *quads;
    u32       *indices;
    
    u32 used;
    u32 size;
};

struct D3D11LineRenderer {

    ID3D11Buffer      *gpuLineBuffer;
    D3D11VertexLine   *cpuLineBuffer;
    D3D11Shader        shader;
    size_t             bufferSize;
    size_t             bufferSizeInBytes;
    size_t             bufferUsed;

    void Initialize(size_t bufferSize, ID3D11Device *device);
    void Terminate();
    void Render(ID3D11DeviceContext *deviceContext);

    void DrawLine(Vec3 a, Vec3 b, u32 color, ID3D11DeviceContext *deviceContext);
private:
    void AddLine(D3D11VertexLine *line);
    Vec4 Vec4Color(u32 color);
    D3D11Shader CreateD3D11Shader(ID3D11Device *device, char *vertpath, char *fragpath);
    void DestroyD3D11Shader(D3D11Shader *shader);
};

struct D3D11BatchRenderer {

    ID3D11Buffer   *gpuVertexBuffer;
    ID3D11Buffer   *gpuIndexBuffer;
    
    D3D11QuadBatch cpuQuadBuffer;

    void Initialize(size_t bufferSize, ID3D11Device *device);
    void Terminate();
    
    void Render(ID3D11DeviceContext *deviceContext);
    void AddBatchVertex(D3D112DVertex *vertices, u32 vertexCount, u32 *indices, u32 indexCount, ID3D11DeviceContext *deviceContext);
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
    ID3D11SamplerState *samplerStateLinear;
    ID3D11SamplerState *samplerStatePoint;

    void Initialize() override;
    void Terminate() override;

    void ResizeBuffers() override;

    void SetRasterizerState(RasterizerState state) override;
    void SetDepthStencilState(bool value) override;
    void SetAlphaBlendState(bool value) override;
    void SetSamplerState(SamplerState state) override;

    void ClearColorBuffer(FrameBuffer frameBufferHandle, f32 r, f32 g, f32 b) override;
    void ClearDepthStencilBuffer(FrameBuffer frameBufferHandle) override;

    void Present(i32 vsync) override;

    Shader CreateShaderVertex(char *vertpath, char *fragpath) override;
    Shader CreateShaderVertexSkin(char *vertpath, char *fragpath) override;
    Shader CreateShaderVertexMap(char *vertpath, char *fragpath) override;
    Shader CreateShaderTGui(char *vertpath, char *fragpath) override;

    void DestroyShader(Shader shaderHandle) override;
    void BindShader(Shader shaderHandle) override;

    ConstBuffer CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName);
    void DestroyConstBuffer(ConstBuffer constBufferHandle);
    void UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData);

    void SetProjMatrix(Mat4 proj)   override; 
    void SetViewMatrix(Mat4 view)   override; 
    void SetWorldMatrix(Mat4 world) override;

    void SetAnimMatrices(Mat4 *finalTransformMatrices, u32 count) override;

    VertexBuffer CreateVertexBuffer(void *vertices, u32 count, size_t stride) override;
    void DestroyVertexBuffer(VertexBuffer vertexBufferHandle) override;
    void DrawVertexBuffer(VertexBuffer vertexBufferHandle, Shader shaderHandle) override;

    IndexBuffer CreateIndexBuffer(u32 *indices, u32 count) override;
    void DestroyIndexBuffer(IndexBuffer indexBuffer) override;
    void DrawIndexBuffer(IndexBuffer indexBuffer, VertexBuffer vertexBuffer, Shader shader) override;

    TextureBuffer CreateTextureBuffer(Texture *array, u32 textureCount) override;
    void DestroyTextureBuffer(TextureBuffer textureBufferHandle) override;
    void BindTextureBuffer(TextureBuffer textureBufferHandle) override;

    void CopyFrameBuffer(FrameBuffer desHandle, FrameBuffer srcHandle) override;
    FrameBuffer CreateWriteFrameBuffer(u32 x, u32 y, u32 width, u32 height) override;
    FrameBuffer CreateReadFrameBuffer(u32 x, u32 y, u32 width, u32 height) override;
    void FrameBufferMap(FrameBuffer frameBufferHandle, u32 *w, u32 *h, u32 *sizeInBytes, u8 **buffer) override;
    void FrameBufferUnmap(FrameBuffer frameBufferHandle) override;
    FrameBuffer CreateFrameBuffer(u32 x, u32 y, u32 width, u32 height) override;
    void DestroyFrameBuffer(FrameBuffer frameBufferHandle) override;
    void BindFrameBuffer(FrameBuffer frameBufferHandle) override;
    TextureBuffer FrameBufferGetTexture(FrameBuffer frameBufferHandle) override;
    void FlushFrameBuffer(FrameBuffer frameBufferHandle) override;

    void SetViewport(u32 x, u32 y, u32 w, u32 h) override;

    void DrawLine(Vec3 a, Vec3 b, u32 color) override;
    void Draw2DBatch(D3D112DVertex *vertices, u32 vertexCount, u32 *indices, u32 indexCount) override;

private:

    ObjectAllocator<D3D11Shader> shadersStorage;
    ObjectAllocator<D3D11ConstBuffer> constBufferStorage;
    ObjectAllocator<D3D11VertexBuffer> vertexBufferStorage;
    ObjectAllocator<D3D11TextureArray> textureArrayStorage;
    ObjectAllocator<D3D11IndexBuffer> indexBufferStorage;
    ObjectAllocator<D3D11FrameBuffer> frameBufferStorage;

    D3D11LineRenderer lineRenderer;
    D3D11BatchRenderer batchRenderer;

};

#endif
