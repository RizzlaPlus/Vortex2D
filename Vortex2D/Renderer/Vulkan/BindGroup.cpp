//
//  DescriptorSet.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D
{
namespace Renderer
{
BindingInput::BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind)
    : Bind(bind), Input(&buffer)

{
}

BindingInput::BindingInput(Renderer::Texture& texture, uint32_t bind)
    : Bind(bind), Input(Image(texture))
{
}

BindingInput::BindingInput(vk::Sampler sampler, Renderer::Texture& texture, uint32_t bind)
    : Bind(bind), Input(Image(sampler, texture))
{
}

Image::Image(vk::Sampler sampler, Renderer::Texture& texture) : Sampler(sampler), Texture(&texture)
{
}

Image::Image(Renderer::Texture& texture) : Sampler(), Texture(&texture) {}

}  // namespace Renderer
}  // namespace Vortex2D
