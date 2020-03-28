//
//  RenderTarget.cpp
//  Vortex2D
//

#include "RenderTarget.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Vortex2D
{
namespace Renderer
{
RenderTarget::RenderTarget(uint32_t width, uint32_t height)
    : Width(width)
    , Height(height)
    , Orth(glm::ortho(0.0f, (float)width, 0.0f, (float)height))
    , View(1.0f)
{
}

RenderTarget::RenderTarget(RenderTarget&& other)
    : Width(other.Width)
    , Height(other.Height)
    , Orth(other.Orth)
    , View(other.View)
    , RenderPass(std::move(other.RenderPass))
{
}

RenderTarget::~RenderTarget() {}

}  // namespace Renderer
}  // namespace Vortex2D
