//
//  Buffer.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>

namespace Vortex2D
{
namespace Renderer
{
void BufferBarrier(vk::Buffer buffer,
                   vk::CommandBuffer command,
                   vk::AccessFlags oldAccess,
                   vk::AccessFlags newAccess)
{
  auto bufferMemoryBarriers = vk::BufferMemoryBarrier()
                                  .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                  .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                  .setBuffer(buffer)
                                  .setSize(VK_WHOLE_SIZE)
                                  .setSrcAccessMask(oldAccess)
                                  .setDstAccessMask(newAccess);

  command.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                          vk::PipelineStageFlagBits::eAllCommands,
                          {},
                          nullptr,
                          bufferMemoryBarriers,
                          nullptr);
}

struct GenericBuffer::Impl
{
  Device& mDevice;
  std::uint64_t mSize;
  vk::BufferUsageFlags mUsageFlags;
  MemoryUsage mMemoryUsage;
  VkBuffer mBuffer;
  VmaAllocation mAllocation;
  VmaAllocationInfo mAllocationInfo;

  Impl(Device& device,
       vk::BufferUsageFlags usageFlags,
       MemoryUsage memoryUsage,
       std::uint64_t deviceSize)
      : mDevice(device)
      , mSize(deviceSize)
      , mUsageFlags(usageFlags | vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eTransferSrc)
      , mMemoryUsage(memoryUsage)
  {
    Create();
  }

  ~Impl()
  {
    if (mBuffer != VK_NULL_HANDLE)
    {
      vmaDestroyBuffer(mDevice.Allocator(), mBuffer, mAllocation);
    }
  }

  Impl(Impl&& other)
      : mDevice(other.mDevice)
      , mSize(other.mSize)
      , mBuffer(other.mBuffer)
      , mAllocation(other.mAllocation)
      , mAllocationInfo(other.mAllocationInfo)
  {
    other.mBuffer = VK_NULL_HANDLE;
    other.mAllocation = VK_NULL_HANDLE;
    other.mSize = 0;
  }

  void Create()
  {
    auto bufferInfo = vk::BufferCreateInfo()
                          .setSize(mSize)
                          .setUsage(mUsageFlags)
                          .setSharingMode(vk::SharingMode::eExclusive);

    VkBufferCreateInfo vkBufferInfo = bufferInfo;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = ConvertMemoryUsage(mMemoryUsage);
    if (vmaCreateBuffer(mDevice.Allocator(),
                        &vkBufferInfo,
                        &allocInfo,
                        &mBuffer,
                        &mAllocation,
                        &mAllocationInfo) != VK_SUCCESS)
    {
      throw std::runtime_error("Error creating buffer");
    }
  }

  vk::Buffer Handle() const { return mBuffer; }

  std::uint64_t Size() const { return mSize; }

  void Resize(std::uint64_t size)
  {
    if (mBuffer != VK_NULL_HANDLE)
    {
      vmaDestroyBuffer(mDevice.Allocator(), mBuffer, mAllocation);
    }

    mSize = size;
    Create();
  }

  void CopyFrom(Renderer::CommandEncoder& command, GenericBuffer& srcBuffer)
  {
    if (mSize != srcBuffer.Size())
    {
      throw std::runtime_error("Cannot copy buffers of different sizes");
    }

    // TODO improve barriers
    srcBuffer.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead);
    Barrier(command, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);

    auto region = vk::BufferCopy().setSize(mSize);

    command.Handle().copyBuffer(srcBuffer.Handle(), mBuffer, region);

    Barrier(command, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
    srcBuffer.Barrier(command, vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eShaderRead);
  }

  void CopyFrom(Renderer::CommandEncoder& command, Texture& srcTexture)
  {
    auto textureSize =
        srcTexture.GetWidth() * srcTexture.GetHeight() * GetBytesPerPixel(srcTexture.GetFormat());
    if (textureSize != mSize)
    {
      throw std::runtime_error("Cannot copy texture of different sizes");
    }

    srcTexture.Barrier(command,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentWrite,
                       vk::ImageLayout::eTransferSrcOptimal,
                       vk::AccessFlagBits::eTransferRead);

    auto info = vk::BufferImageCopy()
                    .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                    .setImageExtent({srcTexture.GetWidth(), srcTexture.GetHeight(), 1});

    command.Handle().copyImageToBuffer(
        srcTexture.Handle(), vk::ImageLayout::eTransferSrcOptimal, mBuffer, info);

    srcTexture.Barrier(command,
                       vk::ImageLayout::eTransferSrcOptimal,
                       vk::AccessFlagBits::eTransferRead,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead);

    Barrier(command, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
  }

  void Barrier(CommandEncoder& command, vk::AccessFlags oldAccess, vk::AccessFlags newAccess)
  {
    BufferBarrier(mBuffer, command.Handle(), oldAccess, newAccess);
  }

  void Clear(Renderer::CommandEncoder& command)
  {
    Barrier(command, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);
    command.Handle().fillBuffer(mBuffer, 0, mSize, 0);
    Barrier(command, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
  }

  void CopyFrom(uint32_t offset, const void* data, uint32_t size)
  {
    // TODO use always mapped functionality of VMA

    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
    if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
      throw std::runtime_error("Not visible buffer");

    void* pData;
    if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
      throw std::runtime_error("Cannot map buffer");

    std::memcpy((uint8_t*)pData + offset, data, size);

    if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
      vmaFlushAllocation(
          mDevice.Allocator(), mAllocation, mAllocationInfo.offset, mAllocationInfo.size);
    }

    vmaUnmapMemory(mDevice.Allocator(), mAllocation);
  }

  void CopyTo(uint32_t offset, void* data, uint32_t size)
  {
    // TODO use always mapped functionality of VMA

    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
    if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
      throw std::runtime_error("Not visible buffer");

    void* pData;
    if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
      throw std::runtime_error("Cannot map buffer");

    if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
      vmaInvalidateAllocation(
          mDevice.Allocator(), mAllocation, mAllocationInfo.offset, mAllocationInfo.size);
    }

    std::memcpy(data, (uint8_t*)pData + offset, size);

    vmaUnmapMemory(mDevice.Allocator(), mAllocation);
  }
};

GenericBuffer::GenericBuffer(Device& device,
                             vk::BufferUsageFlags usageFlags,
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

vk::Buffer GenericBuffer::Handle() const
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

void GenericBuffer::Barrier(CommandEncoder& command,
                            vk::AccessFlags oldAccess,
                            vk::AccessFlags newAccess)
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
