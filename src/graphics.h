#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "common.h"
#include "math.h"

typedef void * Shader;
typedef void * ConstBuffer;
typedef void * VertexBuffer;
typedef void * IndexBuffer;
typedef void * TextureBuffer;

struct TexArray;

enum RasterizerState {
    RASTERIZER_STATE_CULL_BACK,
    RASTERIZER_STATE_CULL_FRONT,
    RASTERIZER_STATE_CULL_NONE,
    RASTERIZER_STATE_WIREFRAME
};  

struct CBMatrix {
    Mat4 proj;
    Mat4 view;
    Mat4 world;
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
    TextureBuffer texture;  
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

struct Graphics {
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;

    virtual void SetRasterizerState(RasterizerState state) = 0;
    virtual void SetDepthStencilState(bool value) = 0;
    virtual void SetAlphaBlendState(bool value) = 0;
    
    virtual void ClearColorBuffer(f32 r, f32 g, f32 b) = 0;
    virtual void ClearDepthStencilBuffer() = 0;
    virtual void Present(i32 vsync) = 0;

    virtual Shader CreateShaderVertex(char *vertpath, char *fragpath) = 0;
    virtual Shader CreateShaderSkinVertex(char *vertpath, char *fragpath) = 0;
    virtual Shader CreateShaderVertexMap(char *vertpath, char *fragpath) = 0;
    virtual void DestroyShader(Shader shaderHandle) = 0;

    virtual ConstBuffer CreateConstBuffer(void *bufferData, u64 bufferSize, u32 index, char *bufferName) = 0;
    virtual void DestroyConstBuffer(ConstBuffer constBufferHandle) = 0;
    virtual void UpdateConstBuffer(ConstBuffer constBufferHandle, void *bufferData) = 0;

    virtual void SetProjMatrix(Mat4 proj) = 0;
    virtual void SetViewMatrix(Mat4 view) = 0;
    virtual void SetWorldMatrix(Mat4 world) = 0;

    virtual void SetAnimMatrices(Mat4 *finalTransformMatrices, u32 count) = 0;

    virtual VertexBuffer CreateVertexBuffer(Vertex *vertices, u32 count) = 0;
    virtual VertexBuffer CreateVertexBuffer(SkinVertex *vertices, u32 count) = 0;
    virtual VertexBuffer CreateVertexBuffer(VertexMap *vertices, u32 count) = 0;
    virtual void DestroyVertexBuffer(VertexBuffer vertexBufferHandle) = 0;
    virtual void DrawVertexBuffer(VertexBuffer vertexBufferHandle, Shader shaderHandle) = 0;

    virtual IndexBuffer CreateIndexBuffer(u32 *indices, u32 count) = 0;
    virtual void DestroyIndexBuffer(IndexBuffer indexBuffer) = 0;
    virtual void DrawIndexBuffer(IndexBuffer indexBuffer, VertexBuffer vertexBuffer, Shader shader) = 0;

    virtual TextureBuffer CreateTextureBuffer(Texture *array, u32 textureCount) = 0;
    virtual void DestroyTextureBuffer(TextureBuffer textureBufferHandle) = 0;
    virtual void BindTextureBuffer(TextureBuffer textureBufferHandle) = 0;

    virtual void DrawLine(Vec3 a, Vec3 b, u32 color) = 0;

    ConstBuffer gpuAnimMatrices;
    CBAnimation cpuAnimMatrices;

    ConstBuffer gpuMatrices; 
    CBMatrix    cpuMatrices;
};

#endif
