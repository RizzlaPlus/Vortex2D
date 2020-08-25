//
//  Instance.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Instance_h
#define Vortex2d_Vulkan_Instance_h

#include <Vortex2D/Renderer/Common.h>

#include "WebGPU.h"

#include <future>
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
  VORTEX2D_API Instance();
  VORTEX2D_API ~Instance();

  WGPUAdapterId GetAdapter() const;

private:
  WGPUAdapterId mAdapter;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
