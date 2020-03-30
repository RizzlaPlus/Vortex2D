//
//  Gpu.h
//  Vortex
//

#ifndef Vortex2D_Vulkan_h
#define Vortex2D_Vulkan_h

#include <Vortex2D/Renderer/Gpu.h>

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

#define VMA_RECORDING_ENABLED 0

#include "vk_mem_alloc.h"

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

namespace Vortex2D
{
namespace Renderer
{
inline bool HasLayer(const char* extension,
                     const std::vector<vk::LayerProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::LayerProperties& layer) {
                       return std::strcmp(extension, layer.layerName) == 0;
                     });
}

inline bool HasExtension(const char* extension,
                         const std::vector<vk::ExtensionProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::ExtensionProperties& layer) {
                       return std::strcmp(extension, layer.extensionName) == 0;
                     });
}

inline VmaMemoryUsage ConvertMemoryUsage(MemoryUsage memoryUsage)
{
  switch (memoryUsage)
  {
    case MemoryUsage::Cpu:
      return VMA_MEMORY_USAGE_CPU_ONLY;
    case MemoryUsage::Gpu:
      return VMA_MEMORY_USAGE_GPU_ONLY;
    case MemoryUsage::GpuToCpu:
      return VMA_MEMORY_USAGE_GPU_TO_CPU;
    case MemoryUsage::CpuToGpu:
      return VMA_MEMORY_USAGE_CPU_TO_GPU;
  }
}

}  // namespace Renderer
}  // namespace Vortex2D

#endif
