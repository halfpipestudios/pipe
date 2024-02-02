#ifndef _D3D11_SHADOW_MAPPING_H_
#define _D3D11_SHADOW_MAPPING_H_

#include <d3d11.h>
#include "common.h"

struct D3D11ShadowMappingBuilder {
    
    D3D11_VIEWPORT viewport;
    ID3D11Texture2D          *depthMap { nullptr };
    ID3D11DepthStencilView   *depthMapDSV  { nullptr };

    ID3D11Texture2D          *readableTexture { nullptr };
    u32 width  { 0 };
    u32 height { 0 };
    
    void Initialize(ID3D11Device *device, u32 width_, u32 height_);
    void Terminate();

    void BindDepthBufferAsRenderTarget(ID3D11DeviceContext *dc);
    void ReadDepthBuffer(ID3D11DeviceContext *dc, u8 **buffer, size_t &size);

};

#endif
