#ifndef _D3D11_PARTICLE_SYS_H_
#define _D3D11_PARTICLE_SYS_H_

#include <d3d11.h>
#include "graphics.h"

#include "common.h"
#include "algebra.h"

#include "mgr/texture_manager.h"

struct VertexParticle {
    Vec3 pos;
    Vec3 vel;
    Vec2 sizeW;
    f32 age;
    u32 type;
};

struct D3D11ParticleSystem {

    void Initialize(ID3D11Device *device, 
                    u32 maxParticles_,
                    Shader soShader_, GeometryShader soGeoShader_,
                    Shader drawShader_, GeometryShader drawGeoShader_,
                    ConstBuffer constBuffer_,
                    Handle texture);

    void Terminate();

    void Reset();
    void Update(Vec3 startPos, Vec3 cameraPos, f32 gameTime_, f32 dt);
    void Draw(ID3D11Device *device, ID3D11DeviceContext *deviceContext);

    u32 maxParticles { 1000 };
    bool firstRun    { true };

    f32 timeStep { 0.0f };
    f32 age      { 0.0f };
    f32 gameTime { 0.0f };

    Vec3 eyePosW  { 0, 0, 0 };
    Vec3 emitPosW { 0, 0, 0 };
    Vec3 emitDirW { 0, 1, 0 };

    ID3D11Buffer *initVB;
    ID3D11Buffer *drawVB;
    ID3D11Buffer *streamOutVB;

    ID3D11ShaderResourceView *randomTexSRV;
    Handle texture;
    
    Shader soShader;
    GeometryShader soGeoShader;
    Shader drawShader;
    GeometryShader drawGeoShader;

    ConstBuffer constBuffer;

private:
    void CreateVertexBuffers(ID3D11Device *device);
    void CreateRandomTexture(ID3D11Device *device);

};

#endif
