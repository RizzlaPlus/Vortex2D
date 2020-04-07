//
//  Gpu.h
//  Vortex
//

#ifndef Vortex2D_Gpu_h
#define Vortex2D_Gpu_h

// temporary
#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

namespace Vortex2D
{
namespace Renderer
{
enum class MemoryUsage
{
  Gpu,
  Cpu,
  CpuToGpu,
  GpuToCpu,
};

enum class Format
{
  R8Uint,
  R8Sint,
  R32Sfloat,
  R32Sint,
  R8G8B8A8Unorm,
  B8G8R8A8Unorm,
  R32G32Sfloat,
  R32G32B32A32Sfloat,
};

enum class ShaderStage
{
  Vertex,
  Fragment,
  Compute,
};

enum class PrimitiveTopology
{
  Triangle,
};

enum class BufferUsage
{
  Vertex,
  Uniform,
  Storage,
  Indirect,
  Index
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
