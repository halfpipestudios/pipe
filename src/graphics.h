#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "common.h"
#include "algebra.h"

// typedef void * Shader;

typedef void * VShader;
typedef void * FShader; 
typedef void * GeometryShader;

typedef void * ConstBuffer;
typedef void * VertexBuffer;
typedef void * IndexBuffer;
typedef void * TextureBuffer;
typedef void * FrameBuffer;
typedef void * ParticleSystem;
typedef void * ShadowMap;


#include "asset_manager.h"

#define SHADOW_MAP_RESOLUTION 2048
#define SHADOW_MAP_FAR_PLANE 50.0f

struct TexArray;

enum RasterizerState {
    RASTERIZER_STATE_CULL_BACK,
    RASTERIZER_STATE_CULL_FRONT,
    RASTERIZER_STATE_CULL_NONE,
    RASTERIZER_STATE_WIREFRAME
};

enum SamplerState {
    SAMPLER_STATE_POINT,
    SAMPLER_STATE_LINEAR
};

enum LightType {
    LIGHT_TYPE_UNDEFINE,
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT
};

struct Light {

    Vec3 pos;
    LightType type;

    Vec3 dir;
    f32 cutOff;

    Vec3 ambient;
    f32 constant;

    Vec3 diffuse;
    f32 linear;

    Vec3 specular;
    f32 quadratic;

    f32 outerCutOff;
    i32 shadowMapIndex;
    f32 pad1;
    f32 pad2;

};

struct Material {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    f32 shininess;
};

#define MAX_LIGHTS_COUNT 64

struct CBLights {
    i32 count;
    Vec3 viewPos;
    Light lights[MAX_LIGHTS_COUNT];
    f32 farPlane;
    f32 pad0;
    f32 pad1;
    f32 pad2;
};

struct CBMaterial {
    Vec3 ambient;
    f32 pad0;
    Vec3 diffuse;
    f32 pad1;
    Vec3 specular;
    f32 shininess;
};

struct CBMatrix {
    Mat4 proj;
    Mat4 view;
    Mat4 world;
};

struct CBTGui {
    i32 res_X;
    i32 res_y;
    u32 padding[2];
};

struct CBGizmo{
    Vec3 color;
    f32 padding[1];
};

struct CBIndex {
    f32 id;
    f32 padding[3];
};

struct CBParticle {
    Vec3 eyePosW;
    f32 gameTime;
    
    Vec3 emitPosW;
    f32 timeStep;
    
    Vec3 emitDirW;
    f32 pad0;

    Vec3 targetPosW;
    f32 pad1;
};

#define MAX_BONES 100
struct CBAnimation {
    Mat4 boneMatrix[MAX_BONES];
};

struct Vertex {
    Vec3 pos;
    Vec3 nor;
    Vec2 uv;
};

struct VertexMap {
    Vec3 pos;
    Vec3 nor;
    Vec2 uv;
    u32 tex;
};

#define MAX_BONES_INFLUENCE 4 
struct SkinVertex : public Vertex {
    i32 boneIds[MAX_BONES_INFLUENCE];
    f32 weights[MAX_BONES_INFLUENCE];
};


struct Texture {
    u32 *pixels;
    i32 w, h;
};

struct Mesh {
    //TextureBuffer texture;  
    Handle texture;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;

    Vertex *vertices;
    u32 numVertices;

    u32 *indices;
    u32 numIndices;

    char material[MAX_NAME_SIZE];
};

enum ModelType {
    MODEL_TYPE_STATIC,
    MODEL_TYPE_ANIMATED,
};

struct Model {
    ModelType type;

    Mesh *meshes;
    u32 numMeshes;
};

struct D3D112DVertex {
    Vec2 pos;
    Vec2 uvs;
    Vec3 color;
};

struct Graphics {
    virtual ~Graphics() = default;
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;

    virtual void ResizeBuffers() = 0;

    virtual void SetRasterizerState(RasterizerState state) = 0;
    virtual void SetShadowMappingRasterState() = 0;
    virtual void SetDepthStencilState(bool value) = 0;
    virtual void SetDepthStencilWriteZeroState(bool value) = 0;
    virtual void SetAlphaBlendState(bool value) = 0;
    virtual void SetAdditiveBlendState(bool value) = 0;
    virtual void SetSamplerState(SamplerState state) = 0;
    
    virtual void ClearColorBuffer(FrameBuffer frameBufferHandle, f32 r, f32 g, f32 b) = 0;
    virtual void ClearDepthStencilBuffer(FrameBuffer frameBufferHandle)  = 0;
    virtual void Present(i32 vsync) = 0;

    virtual VShader CreateVShader(char *vertpath) = 0;
    virtual void DestroyVShader(VShader shaderHandle) = 0;
    virtual void BindVShader(VShader shaderHandle) = 0;

    virtual FShader CreateFShader(char *fragpath) = 0;
    virtual void DestroyFShader(FShader shaderHandle) = 0;
    virtual void BindFShader(FShader shaderHandle) = 0;

    virtual GeometryShader CreateGeometryShader(char *filepath) = 0;
    virtual GeometryShader CreateGeometryShaderWithStreamOutput(char *filepath) = 0;
    virtual void DestroyGeometryShader(GeometryShader geometryShaderHandle) = 0;
    virtual void BindGeometryShader(GeometryShader geometryShaderHandle) = 0;

    virtual void DisablePixelShader() = 0;
    virtual void DisableVertexShader()  = 0;
    virtual void DisableGeometryShader() = 0;

    virtual ConstBuffer CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName) = 0;
    virtual void DestroyConstBuffer(ConstBuffer constBufferHandle) = 0;
    virtual void UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData) = 0;
    virtual void BindConstBuffer(ConstBuffer constBufferHandle, u32 slot) = 0;

    virtual void SetProjMatrix(Mat4 proj) = 0;
    virtual void SetViewMatrix(Mat4 view) = 0;
    virtual void SetWorldMatrix(Mat4 world) = 0;

    // TODO: see if this affect performance
    virtual void SetMaterial(const Material& material) = 0;

    virtual void UpdateLights(Vec3 cameraP, f32 farPlane, Light *lights, i32 count) = 0;


    virtual void SetAnimMatrices(Mat4 *finalTransformMatrices, u32 count) = 0;

    virtual VertexBuffer CreateVertexBuffer(void *vertices, u32 count, size_t stride) = 0;
    virtual void DestroyVertexBuffer(VertexBuffer vertexBufferHandle) = 0;
    virtual void DrawVertexBuffer(VertexBuffer vertexBufferHandle, VShader vshaderHandle, FShader fshaderHandle) = 0;

    virtual IndexBuffer CreateIndexBuffer(u32 *indices, u32 count) = 0;
    virtual void DestroyIndexBuffer(IndexBuffer indexBuffer) = 0;
    virtual void DrawIndexBuffer(IndexBuffer indexBuffer, VertexBuffer vertexBuffer, VShader vshaderHandle, FShader fshaderHandle) = 0;

    virtual TextureBuffer CreateTextureBuffer(Texture *array, u32 textureCount) = 0;
    virtual void DestroyTextureBuffer(TextureBuffer textureBufferHandle) = 0;
    virtual void BindTextureBuffer(TextureBuffer textureBufferHandle) = 0;
    virtual void BindTextureBufferToRegister(TextureBuffer textureBufferHandle, i32 reg) = 0;

    virtual void CopyFrameBuffer(FrameBuffer desHandle, FrameBuffer srcHandle) = 0;
    virtual FrameBuffer CreateWriteFrameBuffer(u32 x, u32 y, u32 width, u32 height) = 0;
    virtual FrameBuffer CreateReadFrameBuffer(u32 x, u32 y, u32 width, u32 height) = 0;
    virtual void FrameBufferMap(FrameBuffer frameBufferHandle, u32 *w, u32 *h, u32 *sizeInBytes, u8 **buffer) = 0;
    virtual void FrameBufferUnmap(FrameBuffer frameBufferHandle) = 0;
    virtual FrameBuffer CreateFrameBuffer(u32 x, u32 y, u32 width, u32 height) = 0;
    virtual void DestroyFrameBuffer(FrameBuffer frameBufferHandle) = 0;
    virtual void BindFrameBuffer(FrameBuffer frameBufferHandle) = 0;
    virtual TextureBuffer FrameBufferGetTexture(FrameBuffer frameBufferHandle) = 0;
    virtual void FlushFrameBuffer(FrameBuffer frameBufferHandle) = 0;

    virtual ParticleSystem CreateParticleSystem(u32 maxParticle,
                                                VShader soVShader, FShader soFShader, GeometryShader soGShader,
                                                VShader dwVShader, FShader dwFShader, GeometryShader dwGShader,
                                                Handle texture) = 0;
    virtual void DestroyParticleSystem(ParticleSystem particleSystemHandle) = 0;
    virtual void ResetParticleSystem(ParticleSystem particleSystemHandle) = 0;
    virtual void UpdateParticleSystem(ParticleSystem particleSystemHandle, Vec3 startPos, Vec3 targetPos, Vec3 cameraPos, f32 gameTime, f32 dt) = 0;
    virtual void RenderParticleSystem(ParticleSystem particleSystemHandle) = 0;

    virtual void SetViewport(u32 x, u32 y, u32 w, u32 h) = 0;

    virtual void DrawLine(Vec3 a, Vec3 b, u32 color) = 0;
    virtual void Draw2DBatch(D3D112DVertex *vertices, u32 vertexCount, u32 *indices, u32 indexCount) = 0;

    virtual void BindDepthBufferAsRenderTarget() = 0;
    virtual void ReadDepthBuffer(u8 **buffer, size_t &size) = 0;

    virtual ShadowMap CreateShadowMap(char *path) = 0;
    virtual void BindShadowMap(ShadowMap shadowMapHandle) = 0;
    virtual void BindShadowMaps(ShadowMap *shadowMapHandles, i32 count) = 0;
    virtual void DestroyShadowMap(ShadowMap shadowMapHandle) = 0;

    ConstBuffer gpuLightsBuffer;
    CBLights    cpuLightsBuffer;

    ConstBuffer gpuMaterialBuffer;
    CBMaterial  cpuMaterialBuffer;

    ConstBuffer gpuAnimMatrices;
    CBAnimation cpuAnimMatrices;

    ConstBuffer gpuMatrices; 
    CBMatrix    cpuMatrices;

    ConstBuffer gpuTGuiBuffer;
    CBTGui cpuTGuiBuffer;

    ConstBuffer gpuGizmoBuffer;
    CBGizmo cpuGizmoBuffer;

    ConstBuffer gpuIndexBuffer;
    CBIndex cpuIndexBuffer;

    ConstBuffer gpuParticleBuffer;
};

#endif
