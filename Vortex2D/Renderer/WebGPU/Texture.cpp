//
//  Texture.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
struct Sampler::Impl
{
  Impl(Device& device, AddressMode addressMode, Filter filter) {}

  Handle::Sampler Handle() { return {}; }
};

Sampler::Sampler(Device& device, AddressMode addressMode, Filter filter)
    : mImpl(std::make_unique<Impl>(device, addressMode, filter))
{
}

Sampler::Sampler(Sampler&& other) : mImpl(std::move(other.mImpl)) {}

Sampler::~Sampler() {}

Handle::Sampler Sampler::Handle()
{
  return mImpl->Handle();
}

struct Texture::Impl
{
  uint32_t mWidth;
  uint32_t mHeight;
  Format mFormat;

  Impl(Device& device, uint32_t width, uint32_t height, Format format, MemoryUsage memoryUsage)
      : mWidth(width), mHeight(height), mFormat(format)
  {
  }

  ~Impl() {}

  Impl(Impl&& other) : mWidth(other.mWidth), mHeight(other.mHeight), mFormat(other.mFormat) {}

  void Clear(CommandEncoder& command, const std::array<int, 4>& colour) {}

  void Clear(CommandEncoder& command, const std::array<float, 4>& colour) {}

  void CopyFrom(const void* data) {}

  void CopyTo(void* data) {}

  void CopyFrom(CommandEncoder& command, Texture& srcImage) {}

  void Barrier(CommandEncoder& command,
               ImageLayout oldLayout,
               Access srcMask,
               ImageLayout newLayout,
               Access dstMask)
  {
  }

  Handle::ImageView GetView() const { return {}; }

  uint32_t GetWidth() const { return mWidth; }

  uint32_t GetHeight() const { return mHeight; }

  Format GetFormat() const { return mFormat; }

  Handle::Image Handle() const { return {}; }
};

Texture::Texture(Device& device,
                 uint32_t width,
                 uint32_t height,
                 Format format,
                 MemoryUsage memoryUsage)
    : mImpl(std::make_unique<Impl>(device, width, height, format, memoryUsage))
{
}

Texture::Texture(Texture&& other) : mImpl(std::move(other.mImpl)) {}

Texture::~Texture() {}

void Texture::CopyFrom(const void* data)
{
  mImpl->CopyFrom(data);
}

void Texture::CopyTo(void* data)
{
  mImpl->CopyTo(data);
}

void Texture::CopyFrom(CommandEncoder& command, Texture& srcImage)
{
  mImpl->CopyFrom(command, srcImage);
}

void Texture::Barrier(CommandEncoder& command,
                      ImageLayout oldLayout,
                      Access oldAccess,
                      ImageLayout newLayout,
                      Access newAccess)
{
  mImpl->Barrier(command, oldLayout, oldAccess, newLayout, newAccess);
}

Handle::ImageView Texture::GetView() const
{
  return mImpl->GetView();
}

uint32_t Texture::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t Texture::GetHeight() const
{
  return mImpl->GetHeight();
}

Format Texture::GetFormat() const
{
  return mImpl->GetFormat();
}

void Texture::Clear(CommandEncoder& command, const std::array<int, 4>& colour)
{
  mImpl->Clear(command, colour);
}

void Texture::Clear(CommandEncoder& command, const std::array<float, 4>& colour)
{
  mImpl->Clear(command, colour);
}

Handle::Image Texture::Handle() const
{
  return mImpl->Handle();
}

}  // namespace Renderer
}  // namespace Vortex2D
