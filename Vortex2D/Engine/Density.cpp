//
//  Density.cpp
//  Vortex2D
//

#include "Density.h"

namespace Vortex2D
{
namespace Fluid
{
Density::Density(Renderer::Device& device, const glm::ivec2& size, Renderer::Format format)
    : Renderer::RenderTexture(device, size.x, size.y, format)
    , Renderer::Sprite(device, *this)
    , mFieldBack(device, size.x, size.y, format)
{
}

}  // namespace Fluid
}  // namespace Vortex2D
