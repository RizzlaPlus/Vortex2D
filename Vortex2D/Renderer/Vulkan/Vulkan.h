//
//  Gpu.h
//  Vortex
//

#ifndef Vortex2D_Vulkan_h
#define Vortex2D_Vulkan_h

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

#define VMA_RECORDING_ENABLED 0

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

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

#endif
