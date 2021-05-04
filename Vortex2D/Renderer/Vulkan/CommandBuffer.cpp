//
//  CommandBuffer.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/CommandBuffer.h>

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/RenderTarget.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
namespace
{
const uint32_t zero = 0;
}

struct CommandEncoder::Impl
{
  Impl(Device& device, vk::UniqueCommandBuffer commandBuffer)
      : mDevice(static_cast<VulkanDevice&>(device)), mCommandBuffer(std::move(commandBuffer))
  {
  }

  void Begin()
  {
    auto bufferBegin =
        vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCommandBuffer->begin(bufferBegin);
  }

  void BeginRenderPass(const RenderTarget& renderTarget, vk::Framebuffer framebuffer)
  {
    auto renderPassBegin = vk::RenderPassBeginInfo()
                               .setFramebuffer(framebuffer)
                               .setRenderPass(*renderTarget.RenderPass)
                               .setRenderArea({{0, 0}, {renderTarget.Width, renderTarget.Height}});

    mCommandBuffer->beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
  }

  void EndRenderPass() { mCommandBuffer->endRenderPass(); }

  void End() { mCommandBuffer->end(); }

  void SetPipeline(PipelineBindPoint pipelineBindPoint, vk::Pipeline pipeline)
  {
    mCommandBuffer->bindPipeline(ConvertPipelineBindPoint(pipelineBindPoint), pipeline);
  }

  void SetBindGroup(PipelineBindPoint pipelineBindPoint,
                    vk::PipelineLayout layout,
                    BindGroup& bindGroup)
  {
    mCommandBuffer->bindDescriptorSets(
        ConvertPipelineBindPoint(pipelineBindPoint), layout, 0, {*bindGroup.descriptorSet}, {});
  }

  void SetVertexBuffer(const GenericBuffer& buffer)
  {
    mCommandBuffer->bindVertexBuffers(0, {buffer.Handle()}, {0ul});
  }

  void PushConstants(vk::PipelineLayout layout,
                     ShaderStage stageFlags,
                     uint32_t offset,
                     uint32_t size,
                     const void* pValues)
  {
    mCommandBuffer->pushConstants(layout, ConvertShaderStage(stageFlags), offset, size, pValues);
  }

  void Draw(std::uint32_t vertexCount) { mCommandBuffer->draw(vertexCount, 1, 0, 0); }

  void Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
  {
    mCommandBuffer->dispatch(x, y, z);
  }

  void DispatchIndirect(GenericBuffer& buffer)
  {
    mCommandBuffer->dispatchIndirect(buffer.Handle(), 0);
  }

  void Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour)
  {
    auto clearValue =
        vk::ClearValue().setColor(std::array<float, 4>{{colour.r, colour.g, colour.b, colour.a}});

    auto clearAttachement = vk::ClearAttachment()
                                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                .setClearValue(clearValue);

    auto clearRect = vk::ClearRect().setRect({{pos.x, pos.y}, {size.x, size.y}}).setLayerCount(1);

    mCommandBuffer->clearAttachments({clearAttachement}, {clearRect});
  }

  void DebugMarkerBegin(const char* name, const glm::vec4& color)
  {
    mCommandBuffer->debugMarkerBeginEXT({name, {{color.r, color.g, color.b, color.a}}},
                                        mDevice.Loader());
  }

  void DebugMarkerEnd() { mCommandBuffer->debugMarkerEndEXT(mDevice.Loader()); }

  vk::CommandBuffer Handle() { return *mCommandBuffer; }

  VulkanDevice& mDevice;
  vk::UniqueCommandBuffer mCommandBuffer;
};

CommandEncoder::CommandEncoder(Device& device, vk::UniqueCommandBuffer commandBuffer)
    : mImpl(std::make_unique<Impl>(device, std::move(commandBuffer)))
{
}

CommandEncoder::CommandEncoder(CommandEncoder&& other) : mImpl(std::move(other.mImpl)) {}

CommandEncoder::~CommandEncoder() {}

void CommandEncoder::Begin()
{
  mImpl->Begin();
}

void CommandEncoder::BeginRenderPass(const RenderTarget& renderTarget, vk::Framebuffer framebuffer)
{
  mImpl->BeginRenderPass(renderTarget, framebuffer);
}

void CommandEncoder::EndRenderPass()
{
  mImpl->EndRenderPass();
}

void CommandEncoder::End()
{
  mImpl->End();
}

void CommandEncoder::SetPipeline(PipelineBindPoint pipelineBindPoint, vk::Pipeline pipeline)
{
  mImpl->SetPipeline(pipelineBindPoint, pipeline);
}

void CommandEncoder::SetBindGroup(PipelineBindPoint pipelineBindPoint,
                                  vk::PipelineLayout layout,
                                  BindGroup& bindGroup)
{
  mImpl->SetBindGroup(pipelineBindPoint, layout, bindGroup);
}

void CommandEncoder::SetVertexBuffer(const GenericBuffer& buffer)
{
  mImpl->SetVertexBuffer(buffer);
}

void CommandEncoder::PushConstants(vk::PipelineLayout layout,
                                   ShaderStage stageFlags,
                                   uint32_t offset,
                                   uint32_t size,
                                   const void* pValues)
{
  mImpl->PushConstants(layout, stageFlags, offset, size, pValues);
}

void CommandEncoder::Draw(std::uint32_t vertexCount)
{
  mImpl->Draw(vertexCount);
}

void CommandEncoder::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
  mImpl->Dispatch(x, y, z);
}

void CommandEncoder::DispatchIndirect(GenericBuffer& buffer)
{
  mImpl->DispatchIndirect(buffer);
}

void CommandEncoder::Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour)
{
  mImpl->Clear(pos, size, colour);
}

void CommandEncoder::DebugMarkerBegin(const char* name, const glm::vec4& color)
{
  mImpl->DebugMarkerBegin(name, color);
}

void CommandEncoder::DebugMarkerEnd()
{
  mImpl->DebugMarkerEnd();
}

vk::CommandBuffer CommandEncoder::Handle()
{
  return mImpl->Handle();
}

struct CommandBuffer::Impl
{
  Impl(Device& device, bool synchronise)
      : mDevice(static_cast<VulkanDevice&>(device))
      , mSynchronise(synchronise)
      , mRecorded(false)
      , mCommandEncoder(device.CreateCommandEncoder())
      , mFence(device.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
  {
  }

  void Record(CommandBuffer::CommandFn commandFn)
  {
    Wait();

    mCommandEncoder.Begin();
    commandFn(mCommandEncoder);
    mCommandEncoder.End();
    mRecorded = true;
  }

  void Record(const RenderTarget& renderTarget, vk::Framebuffer framebuffer, CommandFn commandFn)
  {
    Wait();

    mCommandEncoder.Begin();
    mCommandEncoder.BeginRenderPass(renderTarget, framebuffer);

    commandFn(mCommandEncoder);

    mCommandEncoder.EndRenderPass();
    mCommandEncoder.End();
    mRecorded = true;
  }

  void Wait()
  {
    if (mSynchronise)
    {
      mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
    }
  }

  void Reset()
  {
    if (mSynchronise)
    {
      mDevice.Handle().resetFences({*mFence});
    }
  }

  void Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores,
              const std::initializer_list<vk::Semaphore>& signalSemaphores)
  {
    if (!mRecorded)
      throw std::runtime_error("Submitting a command that wasn't recorded");

    Reset();

    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(),
                                                   vk::PipelineStageFlagBits::eAllCommands);

    vk::CommandBuffer commandBuffers[] = {mCommandEncoder.Handle()};

    auto submitInfo = vk::SubmitInfo()
                          .setCommandBufferCount(1)
                          .setPCommandBuffers(commandBuffers)
                          .setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
                          .setPWaitSemaphores(waitSemaphores.begin())
                          .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
                          .setPSignalSemaphores(signalSemaphores.begin())
                          .setPWaitDstStageMask(waitStages.data());

    if (mSynchronise)
    {
      mDevice.Queue().submit({submitInfo}, *mFence);
    }
    else
    {
      mDevice.Queue().submit({submitInfo}, nullptr);
    }
  }

  bool IsValid() const { return mRecorded; }

  VulkanDevice& mDevice;
  bool mSynchronise;
  bool mRecorded;
  CommandEncoder mCommandEncoder;
  vk::UniqueFence mFence;
};

CommandBuffer::CommandBuffer(Device& device, bool synchronise)
    : mImpl(std::make_unique<Impl>(device, synchronise))
{
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) : mImpl(std::move(other.mImpl)) {}

CommandBuffer::~CommandBuffer() {}

CommandBuffer& CommandBuffer::Record(CommandFn commandFn)
{
  mImpl->Record(commandFn);
  return *this;
}

CommandBuffer& CommandBuffer::Record(const RenderTarget& renderTarget,
                                     vk::Framebuffer framebuffer,
                                     CommandFn commandFn)
{
  mImpl->Record(renderTarget, framebuffer, commandFn);
  return *this;
}

CommandBuffer& CommandBuffer::Wait()
{
  mImpl->Wait();
  return *this;
}

CommandBuffer& CommandBuffer::Reset()
{
  mImpl->Reset();
  return *this;
}

CommandBuffer& CommandBuffer::Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores,
                                     const std::initializer_list<vk::Semaphore>& signalSemaphores)
{
  mImpl->Submit(waitSemaphores, signalSemaphores);
  return *this;
}

CommandBuffer::operator bool() const
{
  return mImpl->IsValid();
}

struct RenderCommand::Impl
{
  Impl(RenderCommand& self) : mSelf(&self), mRenderTarget(nullptr), mIndex(&zero) {}

  Impl(RenderCommand& self,
       Device& device,
       RenderTarget& renderTarget,
       const RenderState& renderState,
       const vk::UniqueFramebuffer& frameBuffer,
       RenderTarget::DrawableList drawables)
      : mSelf(&self)
      , mRenderTarget(&renderTarget)
      , mIndex(&zero)
      , mDrawables(drawables)
      , mView(1.0f)
  {
    for (auto& drawable : drawables)
    {
      drawable.get().Initialize(renderState);
    }

    CommandBuffer cmd(device, true);
    cmd.Record(renderTarget, *frameBuffer, [&](CommandEncoder& command) {
      for (auto& drawable : drawables)
      {
        drawable.get().Draw(command, renderState);
      }
    });

    mCmds.emplace_back(std::move(cmd));
  }

  Impl(RenderCommand& self,
       Device& device,
       RenderTarget& renderTarget,
       const RenderState& renderState,
       const std::vector<vk::UniqueFramebuffer>& frameBuffers,
       const uint32_t& index,
       RenderTarget::DrawableList drawables)
      : mSelf(&self)
      , mRenderTarget(&renderTarget)
      , mIndex(&index)
      , mDrawables(drawables)
      , mView(1.0f)
  {
    for (auto& drawable : drawables)
    {
      drawable.get().Initialize(renderState);
    }

    for (auto& frameBuffer : frameBuffers)
    {
      CommandBuffer cmd(device, true);
      cmd.Record(renderTarget, *frameBuffer, [&](CommandEncoder& command) {
        for (auto& drawable : drawables)
        {
          drawable.get().Draw(command, renderState);
        }
      });

      mCmds.emplace_back(std::move(cmd));
    }
  }

  ~Impl()
  {
    for (auto& cmd : mCmds)
    {
      cmd.Wait();
    }
  }

  void Submit(const glm::mat4& view)
  {
    mView = view;

    if (mRenderTarget)
    {
      mRenderTarget->Submit(*mSelf);
    }
  }

  void Wait()
  {
    assert(mIndex);
    assert(*mIndex < mCmds.size());
    mCmds[*mIndex].Wait();
  }

  void Render(const std::initializer_list<vk::Semaphore>& waitSemaphores,
              const std::initializer_list<vk::Semaphore>& signalSemaphores)
  {
    assert(mIndex);
    if (mCmds.empty())
      return;
    if (*mIndex >= mCmds.size())
      throw std::runtime_error("invalid index");

    for (auto& drawable : mDrawables)
    {
      assert(mRenderTarget);
      drawable.get().Update(mRenderTarget->Orth, mRenderTarget->View * mView);
    }

    mCmds[*mIndex].Wait();
    mCmds[*mIndex].Submit(waitSemaphores, signalSemaphores);
  }

  bool IsValid() const { return !mCmds.empty(); }

  RenderCommand* mSelf;
  RenderTarget* mRenderTarget;
  std::vector<CommandBuffer> mCmds;
  const uint32_t* mIndex;
  std::vector<std::reference_wrapper<Drawable>> mDrawables;
  glm::mat4 mView;
};

RenderCommand::RenderCommand() : mImpl(std::make_unique<Impl>(*this)) {}

RenderCommand::~RenderCommand() {}

RenderCommand::RenderCommand(RenderCommand&& other) : mImpl(std::move(other.mImpl))
{
  mImpl->mSelf = this;
}

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const vk::UniqueFramebuffer& frameBuffer,
                             RenderTarget::DrawableList drawables)
    : mImpl(std::make_unique<Impl>(*this,
                                   device,
                                   renderTarget,
                                   renderState,
                                   std::move(frameBuffer),
                                   drawables))
{
}

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const std::vector<vk::UniqueFramebuffer>& frameBuffers,
                             const uint32_t& index,
                             RenderTarget::DrawableList drawables)
    : mImpl(std::make_unique<Impl>(*this,
                                   device,
                                   renderTarget,
                                   renderState,
                                   std::move(frameBuffers),
                                   index,
                                   drawables))
{
}

RenderCommand& RenderCommand::operator=(RenderCommand&& other)
{
  mImpl = std::move(other.mImpl);
  mImpl->mSelf = this;
  return *this;
}

RenderCommand& RenderCommand::Submit(const glm::mat4& view)
{
  mImpl->Submit(view);
  return *this;
}

void RenderCommand::Wait()
{
  mImpl->Wait();
}

void RenderCommand::Render(const std::initializer_list<vk::Semaphore>& waitSemaphores,
                           const std::initializer_list<vk::Semaphore>& signalSemaphores)
{
  mImpl->Render(waitSemaphores, signalSemaphores);
}

RenderCommand::operator bool() const
{
  return mImpl->IsValid();
}

}  // namespace Renderer
}  // namespace Vortex2D
