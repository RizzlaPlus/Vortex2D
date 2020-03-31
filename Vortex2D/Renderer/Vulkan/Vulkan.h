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

inline vk::Format ConvertFormat(Format format)
{
  switch (format)
  {
    case Format::R8Uint:
      return vk::Format::eR8Uint;
    case Format::R8Sint:
      return vk::Format::eR8Sint;
    case Format::R32Sfloat:
      return vk::Format::eR32Sfloat;
    case Format::R32Sint:
      return vk::Format::eR32Sint;
    case Format::R8G8B8A8Unorm:
      return vk::Format::eR8G8B8A8Unorm;
    case Format::B8G8R8A8Unorm:
      return vk::Format::eB8G8R8A8Unorm;
    case Format::R32G32Sfloat:
      return vk::Format::eR32G32Sfloat;
    case Format::R32G32B32A32Sfloat:
      return vk::Format::eR32G32B32A32Sfloat;
  }
}

}  // namespace Renderer
}  // namespace Vortex2D

#endif
