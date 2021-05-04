//
//  RenderWindow.h
//  Vortex2D
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Render to a swapchain, i.e. to the window/surface.
 */
class RenderWindow : public RenderTarget
{
public:
  /**
   * @brief Initialize with a given surface and size.
   * @param device vulkan device
   * @param surface vulkan surface
   * @param width
   * @param height
   */
  VORTEX2D_API RenderWindow(Device& device,
                            Handle::Surface surface,
                            uint32_t width,
                            uint32_t height);

  VORTEX2D_API ~RenderWindow() override;

  VORTEX2D_API RenderCommand Record(DrawableList drawables,
                                    ColorBlendState blendState = {}) override;
  VORTEX2D_API void Submit(RenderCommand& renderCommand) override;

  /**
   * @brief Submits all the render command and present the surface for display.
   */
  VORTEX2D_API void Display();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
