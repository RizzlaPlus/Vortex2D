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

enum class Access
{
  None,
  Write,
  Read,
};

enum class ImageLayout
{
  General,
};

enum class PipelineBindPoint
{
  Graphics,
  Compute
};

enum class BlendFactor
{
  Zero,
  One,
  ConstantColor,
  SrcAlpha,
  OneMinusSrcAlpha,
};

enum class BlendOp
{
  Add,
  Max,
  Min
};

namespace Handle
{
// FIXME doesn't work for 32 bit
typedef struct PipelineLayout_T* PipelineLayout;
typedef struct BindGroupLayout_T* BindGroupLayout;
typedef struct Pipeline_T* Pipeline;
typedef struct ShaderModule_T* ShaderModule;
typedef struct RenderPass_T* RenderPass;
typedef struct Framebuffer_T* Framebuffer;
typedef struct CommandBuffer_T* CommandBuffer;
typedef struct Semaphore_T* Semaphore;
typedef struct BindGroup_T* BindGroup;
}  // namespace Handle

}  // namespace Renderer
}  // namespace Vortex2D

#endif
