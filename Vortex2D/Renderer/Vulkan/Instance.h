//
//  Instance.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Instance_h
#define Vortex2d_Vulkan_Instance_h

#include <Vortex2D/Renderer/Common.h>
#include "Vulkan.h"

#include <memory>
#include <string>
#include <vector>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Vulkan instance, which extensions enabled.
 */
class Instance
{
public:
  VORTEX2D_API Instance(const std::string& name,
                        std::vector<const char*> extensions,
                        bool validation);
  VORTEX2D_API ~Instance();

  VORTEX2D_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX2D_API vk::Instance GetInstance() const;

private:
  vk::UniqueInstance mInstance;
  vk::DispatchLoaderDynamic mLoader;
  vk::PhysicalDevice mPhysicalDevice;
  vk::DebugReportCallbackEXT mDebugCallback;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
