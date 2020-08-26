//
//  Gpu.h
//  Vortex
//

#ifndef Vortex2D_WebGPU_h
#define Vortex2D_WebGPU_h

#include <Vortex2D/Renderer/Gpu.h>

extern "C"
{
#include <wgpu.h>
}

namespace Vortex2D
{
namespace Renderer
{
inline WGPUBufferUsage ConvertBufferUsage(BufferUsage usage)
{
  switch (usage)
  {
    case BufferUsage::Indirect:
      return WGPUBufferUsage_INDIRECT;
    case BufferUsage::Vertex:
      return WGPUBufferUsage_VERTEX;
    case BufferUsage::Uniform:
      return WGPUBufferUsage_UNIFORM;
    case BufferUsage::Storage:
      return WGPUBufferUsage_STORAGE;
    case BufferUsage::Index:
      return WGPUBufferUsage_INDEX;
  }
}
namespace Handle
{
inline WGPUBufferId ConvertBuffer(Buffer buffer)
{
  return reinterpret_cast<WGPUBufferId>(buffer);
}
inline WGPUCommandEncoderId ConvertCommandBuffer(CommandBuffer commandBuffer)
{
  return reinterpret_cast<WGPUCommandEncoderId>(commandBuffer);
}
inline WGPUTextureId ConvertImage(Image image)
{
  return reinterpret_cast<WGPUTextureId>(image);
}
}  // namespace Handle
}  // namespace Renderer
}  // namespace Vortex2D

#endif
