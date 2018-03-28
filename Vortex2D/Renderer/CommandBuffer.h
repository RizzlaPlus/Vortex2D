//
//  CommandBuffer.h
//  Vortex2D
//

#ifndef Vortex2d_CommandBuffer_h
#define Vortex2d_CommandBuffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderTarget.h>

#include <vector>
#include <functional>
#include <initializer_list>

namespace Vortex2D { namespace Renderer {

class CommandBuffer
{
public:
    using CommandFn = std::function<void(vk::CommandBuffer)>;

    CommandBuffer(const Device& device, bool synchronise = true);
    ~CommandBuffer();

    CommandBuffer(CommandBuffer&&);
    CommandBuffer& operator=(CommandBuffer&&);

    void Record(CommandFn commandFn);
    void Record(const RenderTarget& renderTarget, vk::Framebuffer framebuffer, CommandFn commandFn);
    void Wait();
    void Reset();
    void Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores = {},
                const std::initializer_list<vk::Semaphore>& signalSemaphores = {});

    explicit operator bool() const;

private:
    const Device& mDevice;
    bool mSynchronise;
    bool mRecorded;
    vk::CommandBuffer mCommandBuffer;
    vk::UniqueFence mFence;
};

void ExecuteCommand(const Device& device, CommandBuffer::CommandFn commandFn);

class RenderCommand
{
public:
    RenderCommand();
    ~RenderCommand();

    RenderCommand(RenderCommand&&);
    RenderCommand& operator=(RenderCommand&&);

    void Submit(const glm::mat4& view = glm::mat4());

    friend class RenderTexture;
    friend class RenderWindow;

private:
    RenderCommand(const Device& device,
                  RenderTarget& renderTarget,
                  const RenderState& renderState,
                  const vk::UniqueFramebuffer& frameBuffer,
                  RenderTarget::DrawableList drawables);

    RenderCommand(const Device& device,
                  RenderTarget& renderTarget,
                  const RenderState& renderState,
                  const std::vector<vk::UniqueFramebuffer>& frameBuffers,
                  const uint32_t& index,
                  RenderTarget::DrawableList drawables);

    void Render(const std::initializer_list<vk::Semaphore>& waitSemaphores = {},
                const std::initializer_list<vk::Semaphore>& signalSemaphores = {});

    RenderTarget* mRenderTarget;
    std::vector<CommandBuffer> mCmds;
    const uint32_t* mIndex;
    std::vector<std::reference_wrapper<Drawable>> mDrawables;
    glm::mat4 mView;
};



}}

#endif
