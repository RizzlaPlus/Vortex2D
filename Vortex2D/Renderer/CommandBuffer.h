//
//  CommandBuffer.h
//  Vortex2D
//

#ifndef Vortex2d_CommandBuffer_h
#define Vortex2d_CommandBuffer_h

#include <Vortex2D/Renderer/BindGroup.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderTarget.h>

#include <functional>
#include <initializer_list>
#include <vector>

namespace Vortex2D
{
namespace Renderer
{
class Device;
class GenericBuffer;

class CommandEncoder
{
public:
  CommandEncoder(Device& device);
  CommandEncoder(CommandEncoder&&);
  ~CommandEncoder();

  void Begin();
  void BeginRenderPass(const RenderTarget& renderTarget, Handle::Framebuffer framebuffer);
  void EndRenderPass();
  void End();

  VORTEX2D_API void SetPipeline(PipelineBindPoint pipelineBindPoint, Handle::Pipeline pipeline);

  VORTEX2D_API void SetBindGroup(PipelineBindPoint pipelineBindPoint,
                                 Handle::PipelineLayout layout,
                                 BindGroup& bindGroup);

  VORTEX2D_API void SetVertexBuffer(const GenericBuffer& buffer);

  VORTEX2D_API void PushConstants(Handle::PipelineLayout layout,
                                  ShaderStage stageFlags,
                                  uint32_t offset,
                                  uint32_t size,
                                  const void* pValues);

  VORTEX2D_API void Draw(std::uint32_t vertexCount);
  VORTEX2D_API void Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z);
  VORTEX2D_API void DispatchIndirect(GenericBuffer& buffer);

  VORTEX2D_API void Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour);

  VORTEX2D_API void DebugMarkerBegin(const char* name, const glm::vec4& color);
  VORTEX2D_API void DebugMarkerEnd();

  Handle::CommandBuffer Handle();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

/**
 * @brief Can record commands, then submit them (multiple times).
 * A fence can used to wait on the completion of the commands.
 */
class CommandBuffer
{
public:
  using CommandFn = std::function<void(CommandEncoder&)>;

  /**
   * @brief Creates a command buffer which can be synchronized.
   * @param device vulkan device
   * @param synchronise flag to determine if the command buffer can be waited
   * on.
   */
  VORTEX2D_API explicit CommandBuffer(Device& device, bool synchronise = true);

  VORTEX2D_API CommandBuffer(CommandBuffer&&);

  VORTEX2D_API ~CommandBuffer();

  /**
   * @brief Record some commands. The commads are recorded in the lambda which
   * is immediately executed.
   * @param commandFn a functor, or simply a lambda, where commands are
   * recorded.
   */
  VORTEX2D_API CommandBuffer& Record(CommandFn commandFn);

  /**
   * @brief Record some commands inside a render pass. The commads are recorded
   * in the lambda which is immediately executed.
   * @param renderTarget the render target which contains the render pass to
   * record into
   * @param framebuffer the frame buffer where the render pass will render.
   * @param commandFn a functor, or simply a lambda, where commands are
   * recorded.
   */
  VORTEX2D_API CommandBuffer& Record(const RenderTarget& renderTarget,
                                     Handle::Framebuffer framebuffer,
                                     CommandFn commandFn);

  /**
   * @brief Wait for the command submit to finish. Does nothing if the
   * synchronise flag was false.
   */
  VORTEX2D_API CommandBuffer& Wait();

  /**
   * @brief Reset the command buffer so it can be recorded again.
   */
  VORTEX2D_API CommandBuffer& Reset();

  /**
   * @brief submit the command buffer
   */
  VORTEX2D_API CommandBuffer& Submit(
      const std::initializer_list<Handle::Semaphore>& waitSemaphores = {},
      const std::initializer_list<Handle::Semaphore>& signalSemaphores = {});

  /**
   * @brief explicit conversion operator to bool, indicates if the command was
   * properly recorded and can be sumitted.
   */
  VORTEX2D_API explicit operator bool() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

/**
 * @brief A special command buffer that has been recorded by a @ref
 * RenderTarget. It can be used to submit the rendering. The object has to stay
 * alive untill rendering is complete.
 */
class RenderCommand
{
public:
  VORTEX2D_API RenderCommand();
  VORTEX2D_API ~RenderCommand();

  VORTEX2D_API RenderCommand(RenderCommand&&);
  VORTEX2D_API RenderCommand& operator=(RenderCommand&&);

  VORTEX2D_API RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const Handle::Framebuffer& frameBuffer,
                             RenderTarget::DrawableList drawables);

  VORTEX2D_API RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const std::vector<Handle::Framebuffer>& frameBuffers,
                             const uint32_t& index,
                             RenderTarget::DrawableList drawables);

  /**
   * @brief Submit the render command with a transform matrix
   * @param view a transform matrix
   * @return *this
   */
  VORTEX2D_API RenderCommand& Submit(const glm::mat4& view = glm::mat4(1.0f));

  /**
   * @brief Wait for the render command to complete
   */
  VORTEX2D_API void Wait();

  VORTEX2D_API void Render(const std::initializer_list<Handle::Semaphore>& waitSemaphores = {},
                           const std::initializer_list<Handle::Semaphore>& signalSemaphores = {});

  /**
   * @brief explicit conversion operator to bool, indicates if the command was
   * properly recorded and can be sumitted.
   */
  VORTEX2D_API explicit operator bool() const;

  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
