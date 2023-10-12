#include "graphics_manager.h"

D3D11Graphics GraphicsManager::d3d11;

Graphics *GraphicsManager::Get()
{
    return (Graphics *)&d3d11;
}
