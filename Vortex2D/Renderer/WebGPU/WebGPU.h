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

inline WGPUBufferUsage ConvertMemoryUsage(MemoryUsage usage)
{
  switch (usage)
  {
    case MemoryUsage::Cpu:
    case MemoryUsage::CpuToGpu:
    case MemoryUsage::GpuToCpu:
      return WGPUBufferUsage_MAP_READ | WGPUBufferUsage_MAP_WRITE | WGPUBufferUsage_COPY_DST |
             WGPUBufferUsage_COPY_SRC;
    case MemoryUsage::Gpu:
      return WGPUBufferUsage_COPY_DST | WGPUBufferUsage_COPY_SRC;
  }
}

inline WGPUBindingType ConvertBindingType(BindType type)
{
  switch (type)
  {
    case BindType::StorageBuffer:
      return WGPUBindingType_StorageBuffer;
    case BindType::StorageImage:
      return WGPUBindingType_WriteonlyStorageTexture;
    case BindType::ImageSampler:
      return WGPUBindingType_SampledTexture;
    case BindType::UniformBuffer:
      return WGPUBindingType_UniformBuffer;
  }
}

inline WGPUShaderStage ConvertShaderStage(ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::Compute:
      return WGPUShaderStage_COMPUTE;
    case ShaderStage::Fragment:
      return WGPUShaderStage_FRAGMENT;
    case ShaderStage::Vertex:
      return WGPUShaderStage_VERTEX;
  }
}

namespace Handle
{
inline WGPUBufferId ConvertBuffer(Buffer buffer)
{
  return reinterpret_cast<WGPUBufferId>(buffer);
}
inline WGPUCommandEncoderId ConvertCommandEncoder(CommandEncoder commandEncoder)
{
  return reinterpret_cast<WGPUCommandEncoderId>(commandEncoder);
}
inline WGPUCommandBufferId ConvertCommandBuffer(CommandBuffer commandBuffer)
{
  return reinterpret_cast<WGPUCommandBufferId>(commandBuffer);
}
inline WGPUTextureId ConvertImage(Image image)
{
  return reinterpret_cast<WGPUTextureId>(image);
}
inline WGPUBindGroupLayoutId ConvertBindGroupLayout(BindGroupLayout layout)
{
  return reinterpret_cast<WGPUBindGroupLayoutId>(layout);
}
}  // namespace Handle
}  // namespace Renderer
}  // namespace Vortex2D

#endif
