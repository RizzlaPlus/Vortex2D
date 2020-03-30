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

}  // namespace Renderer
}  // namespace Vortex2D

#endif
