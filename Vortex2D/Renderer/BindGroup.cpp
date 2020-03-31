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

BindingInput::BindingInput(Sampler& sampler, Renderer::Texture& texture, uint32_t bind)
    : Bind(bind), Input(Image(texture, sampler))
{
}

Image::Image(Renderer::Texture& texture, Renderer::Sampler& sampler)
    : Texture(&texture), Sampler(&sampler)
{
}

Image::Image(Renderer::Texture& texture) : Texture(&texture), Sampler(nullptr) {}

}  // namespace Renderer
}  // namespace Vortex2D
