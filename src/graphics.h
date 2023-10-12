#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "common.h"

typedef u32 Shader;

enum RasterizerState {
    RASTERIZER_STATE_CULL_BACK,
    RASTERIZER_STATE_CULL_FRONT,
    RASTERIZER_STATE_CULL_NONE,
    RASTERIZER_STATE_WIREFRAME
};  

struct Graphics {
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;

    virtual void SetRasterizerState(RasterizerState state) = 0;
    virtual void SetDepthStencilState(bool value) = 0;
    virtual void SetAlphaBlendState(bool value) = 0;
    
    virtual void ClearColorBuffer(f32 r, f32 g, f32 b) = 0;
    virtual void ClearDepthStencilBuffer() = 0;
    virtual void Present(i32 vsync) = 0;

    virtual Shader CreateShader(char *vertpath, char *fragpath) = 0;
    virtual void DestroyShader(Shader shaderHandle) = 0;

};

#endif
