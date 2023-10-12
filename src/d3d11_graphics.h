#ifndef _D3D11_GRAPHICS_H_
#define _D3D11_GRAPHICS_H_

#include <d3d11.h>
#include <d3dcompiler.h>

#include "graphics.h"

struct D3D11Shader {
    ID3D11VertexShader *vertex;
    ID3D11PixelShader *fragment;
    ID3DBlob *vertexShaderCompiled;
    ID3DBlob *fragmentShaderCompiled;
};

struct D3D11ShaderStorage {
    D3D11Shader shaders[256];
    i32 shadersCount;
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

    void Initialize() override;
    void Terminate() override;

    void SetRasterizerState(RasterizerState state) override;
    void SetDepthStencilState(bool value) override;
    void SetAlphaBlendState(bool value) override;

    void ClearColorBuffer(f32 r, f32 g, f32 b) override;
    void ClearDepthStencilBuffer() override;
    void Present(i32 vsync) override;

    Shader CreateShader(char *vertpath, char *fragpath) override;
    void DestroyShader(Shader shaderHandle) override;

    static D3D11ShaderStorage shadersStorage;

};

#endif
