//
//  RenderTarget.h
//  Vortex2D
//

#ifndef Vortex2D_RenderTarget_h
#define Vortex2D_RenderTarget_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <functional>
#include <initializer_list>

namespace Vortex2D
{
namespace Renderer
{
class RenderCommand;
class Device;
struct Drawable;

/**
 * @brief A target that can be rendered to.
 * This is implemented by the @ref RenderWindow and the @ref RenderTexture
 */
struct RenderTarget
{
  VORTEX2D_API RenderTarget(Device& device,
                            uint32_t width,
                            uint32_t height,
                            Handle::RenderPass renderPass);

  RenderTarget(RenderTarget&& other);

  VORTEX2D_API virtual ~RenderTarget();

  // TODO should use shared_ptr?
  using DrawableList = std::initializer_list<std::reference_wrapper<Drawable>>;

  VORTEX2D_API virtual RenderCommand Record(DrawableList drawables,
                                            ColorBlendState blendState = {}) = 0;

  // TODO should use shared_ptr?
  VORTEX2D_API virtual void Submit(RenderCommand& renderCommand) = 0;

  uint32_t GetWidth() const;

  uint32_t GetHeight() const;

  const glm::mat4& GetOrth() const;

  const glm::mat4& GetView() const;

  VORTEX2D_API void SetView(const glm::mat4& view);

  Handle::RenderPass GetRenderPass() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
