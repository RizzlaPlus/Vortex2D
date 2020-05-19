//
//  DescriptorSet.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
struct BindGroup::Impl
{
  Impl(Device& device, const Handle::BindGroupLayout& bindGroupLayout)
      : mDescriptorSet(static_cast<VulkanDevice&>(device).CreateDescriptorSet(
            reinterpret_cast<VkDescriptorSetLayout>(bindGroupLayout)))
  {
  }

  Handle::BindGroup Handle()
  {
    return reinterpret_cast<Handle::BindGroup>(static_cast<VkDescriptorSet>(*mDescriptorSet));
  }

  vk::UniqueDescriptorSet mDescriptorSet;
};

BindGroup::BindGroup(Device& device, const Handle::BindGroupLayout& bindGroupLayout)
    : mImpl(std::make_unique<Impl>(device, bindGroupLayout))
{
}

BindGroup::BindGroup() {}

BindGroup::BindGroup(BindGroup&& other) : mImpl(std::move(other.mImpl)) {}

BindGroup::~BindGroup() {}

BindGroup& BindGroup::operator=(BindGroup&& other)
{
  mImpl = std::move(other.mImpl);
  return *this;
}

Handle::BindGroup BindGroup::Handle()
{
  assert(mImpl);
  return mImpl->Handle();
}

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
