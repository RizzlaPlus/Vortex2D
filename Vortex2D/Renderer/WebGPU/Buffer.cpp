//
//  Buffer.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
struct GenericBuffer::Impl
{
  std::uint64_t mSize;
  MemoryUsage mMemoryUsage;

  Impl(Device& device, BufferUsage usageFlags, MemoryUsage memoryUsage, std::uint64_t deviceSize)
      : mSize(deviceSize), mMemoryUsage(memoryUsage)
  {
  }

  ~Impl() {}

  Impl(Impl&& other) : mSize(other.mSize) { other.mSize = 0; }

  Handle::Buffer Handle() const { return {}; }

  std::uint64_t Size() const { return mSize; }

  void Resize(std::uint64_t size) {}

  void CopyFrom(Renderer::CommandEncoder& command, GenericBuffer& srcBuffer)
  {
    if (mSize != srcBuffer.Size())
    {
      throw std::runtime_error("Cannot copy buffers of different sizes");
    }
  }

  void CopyFrom(Renderer::CommandEncoder& command, Texture& srcTexture)
  {
    auto textureSize =
        srcTexture.GetWidth() * srcTexture.GetHeight() * GetBytesPerPixel(srcTexture.GetFormat());
    if (textureSize != mSize)
    {
      throw std::runtime_error("Cannot copy texture of different sizes");
    }
  }

  void Barrier(CommandEncoder& command, Access oldAccess, Access newAccess) {}

  void Clear(Renderer::CommandEncoder& command) {}

  void CopyFrom(uint32_t offset, const void* data, uint32_t size) {}

  void CopyTo(uint32_t offset, void* data, uint32_t size) {}
};

GenericBuffer::GenericBuffer(Device& device,
                             BufferUsage usageFlags,
                             MemoryUsage memoryUsage,
                             std::uint64_t deviceSize)
    : mImpl(std::make_unique<GenericBuffer::Impl>(device, usageFlags, memoryUsage, deviceSize))
{
}

GenericBuffer::GenericBuffer(GenericBuffer&& other) : mImpl(std::move(other.mImpl)) {}

GenericBuffer::~GenericBuffer() {}

void GenericBuffer::CopyFrom(Renderer::CommandEncoder& command, GenericBuffer& srcBuffer)
{
  mImpl->CopyFrom(command, srcBuffer);
}

void GenericBuffer::CopyFrom(Renderer::CommandEncoder& command, Texture& srcTexture)
{
  mImpl->CopyFrom(command, srcTexture);
}

Handle::Buffer GenericBuffer::Handle() const
{
  return mImpl->Handle();
}

std::uint64_t GenericBuffer::Size() const
{
  return mImpl->Size();
}

void GenericBuffer::Resize(std::uint64_t size)
{
  mImpl->Resize(size);
}

void GenericBuffer::Barrier(CommandEncoder& command, Access oldAccess, Access newAccess)
{
  mImpl->Barrier(command, oldAccess, newAccess);
}

void GenericBuffer::Clear(Renderer::CommandEncoder& command)
{
  mImpl->Clear(command);
}

void GenericBuffer::CopyFrom(uint32_t offset, const void* data, uint32_t size)
{
  mImpl->CopyFrom(offset, data, size);
}

void GenericBuffer::CopyTo(uint32_t offset, void* data, uint32_t size)
{
  mImpl->CopyTo(offset, data, size);
}

}  // namespace Renderer
}  // namespace Vortex2D
