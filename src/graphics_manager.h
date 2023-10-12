#ifndef _GRAPHICS_MANAGER_H_
#define _GRAPHICS_MANAGER_H_

#include "d3d11_graphics.h"

struct GraphicsManager {
    static Graphics *Get();
private:
    static D3D11Graphics d3d11;
    // OpenglGraphics opengl;
    // VulkanGraphics vulkan;
    // D3D12Graphics  d3d12;
};

#endif
