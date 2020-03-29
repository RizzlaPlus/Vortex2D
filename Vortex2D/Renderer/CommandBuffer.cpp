//
//  CommandBuffer.cpp
//  Vortex2D
//

#include "CommandBuffer.h"

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D
{
namespace Renderer
{
namespace
{
const uint32_t zero = 0;

}

CommandEncoder::CommandEncoder(Device& device, vk::UniqueCommandBuffer commandBuffer)
    : mDevice(&device), mCommandBuffer(std::move(commandBuffer))
{
}

CommandEncoder::CommandEncoder(CommandEncoder&& command)
    : mDevice(command.mDevice), mCommandBuffer(std::move(command.mCommandBuffer))
{
}

CommandEncoder& CommandEncoder::operator=(CommandEncoder&& command)
{
  mDevice = command.mDevice;
  mCommandBuffer = std::move(command.mCommandBuffer);

  return *this;
}

void CommandEncoder::Begin()
{
  auto bufferBegin =
      vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

  mCommandBuffer->begin(bufferBegin);
}

void CommandEncoder::BeginRenderPass(const RenderTarget& renderTarget, vk::Framebuffer framebuffer)
{
  auto renderPassBegin = vk::RenderPassBeginInfo()
                             .setFramebuffer(framebuffer)
                             .setRenderPass(*renderTarget.RenderPass)
                             .setRenderArea({{0, 0}, {renderTarget.Width, renderTarget.Height}});

  mCommandBuffer->beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
}

void CommandEncoder::EndRenderPass()
{
  mCommandBuffer->endRenderPass();
}

void CommandEncoder::End()
{
  mCommandBuffer->end();
}

void CommandEncoder::SetPipeline(vk::PipelineBindPoint pipelineBindPoint, vk::Pipeline pipeline)
{
  mCommandBuffer->bindPipeline(pipelineBindPoint, pipeline);
}

void CommandEncoder::SetBindGroup(vk::PipelineBindPoint pipelineBindPoint,
                                  vk::PipelineLayout layout,
                                  BindGroup& bindGroup)
{
  mCommandBuffer->bindDescriptorSets(pipelineBindPoint, layout, 0, {*bindGroup.descriptorSet}, {});
}

void CommandEncoder::SetVertexBuffer(const GenericBuffer& buffer)
{
  mCommandBuffer->bindVertexBuffers(0, {buffer.Handle()}, {0ul});
}

void CommandEncoder::PushConstants(vk::PipelineLayout layout,
                                   vk::ShaderStageFlags stageFlags,
                                   uint32_t offset,
                                   uint32_t size,
                                   const void* pValues)
{
  mCommandBuffer->pushConstants(layout, stageFlags, offset, size, pValues);
}

void CommandEncoder::Draw(std::uint32_t vertexCount)
{
  mCommandBuffer->draw(vertexCount, 1, 0, 0);
}

void CommandEncoder::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
  mCommandBuffer->dispatch(x, y, z);
}

void CommandEncoder::DispatchIndirect(GenericBuffer& buffer)
{
  mCommandBuffer->dispatchIndirect(buffer.Handle(), 0);
}

void CommandEncoder::Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour)
{
  auto clearValue =
      vk::ClearValue().setColor(std::array<float, 4>{{colour.r, colour.g, colour.b, colour.a}});

  auto clearAttachement = vk::ClearAttachment()
                              .setAspectMask(vk::ImageAspectFlagBits::eColor)
                              .setClearValue(clearValue);

  auto clearRect = vk::ClearRect().setRect({{pos.x, pos.y}, {size.x, size.y}}).setLayerCount(1);

  mCommandBuffer->clearAttachments({clearAttachement}, {clearRect});
}

void CommandEncoder::DebugMarkerBegin(const char* name, const glm::vec4& color)
{
  mCommandBuffer->debugMarkerBeginEXT({name, {{color.r, color.g, color.b, color.a}}},
                                      mDevice->Loader());
}

void CommandEncoder::DebugMarkerEnd()
{
  mCommandBuffer->debugMarkerEndEXT(mDevice->Loader());
}

vk::CommandBuffer CommandEncoder::Handle()
{
  return *mCommandBuffer;
}

CommandBuffer::CommandBuffer(Device& device, bool synchronise)
    : mDevice(device)
    , mSynchronise(synchronise)
    , mRecorded(false)
    , mCommandEncoder(device.CreateCommandEncoder())
    , mFence(device.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
{
}

CommandBuffer::CommandBuffer(CommandBuffer&& other)
    : mDevice(other.mDevice)
    , mSynchronise(other.mSynchronise)
    , mRecorded(other.mRecorded)
    , mCommandEncoder(std::move(other.mCommandEncoder))
    , mFence(std::move(other.mFence))
{
  other.mRecorded = false;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other)
{
  assert(mDevice.Handle() == other.mDevice.Handle());
  mSynchronise = other.mSynchronise;
  mRecorded = other.mRecorded;
  mCommandEncoder = std::move(other.mCommandEncoder);
  mFence = std::move(other.mFence);

  other.mRecorded = false;

  return *this;
}

CommandBuffer& CommandBuffer::Record(CommandBuffer::CommandFn commandFn)
{
  Wait();

  mCommandEncoder.Begin();
  commandFn(mCommandEncoder);
  mCommandEncoder.End();
  mRecorded = true;

  return *this;
}

CommandBuffer& CommandBuffer::Record(const RenderTarget& renderTarget,
                                     vk::Framebuffer framebuffer,
                                     CommandFn commandFn)
{
  Wait();

  mCommandEncoder.Begin();
  mCommandEncoder.BeginRenderPass(renderTarget, framebuffer);

  commandFn(mCommandEncoder);

  mCommandEncoder.EndRenderPass();
  mCommandEncoder.End();
  mRecorded = true;

  return *this;
}

CommandBuffer& CommandBuffer::Wait()
{
  if (mSynchronise)
  {
    mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
  }

  return *this;
}

CommandBuffer& CommandBuffer::Reset()
{
  if (mSynchronise)
  {
    mDevice.Handle().resetFences({*mFence});
  }

  return *this;
}

CommandBuffer& CommandBuffer::Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores,
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

  return *this;
}

CommandBuffer::operator bool() const
{
  return mRecorded;
}

RenderCommand::RenderCommand(RenderCommand&& other)
    : mRenderTarget(other.mRenderTarget)
    , mCmds(std::move(other.mCmds))
    , mIndex(other.mIndex)
    , mDrawables(std::move(other.mDrawables))
    , mView(other.mView)
{
  other.mRenderTarget = nullptr;
  other.mIndex = nullptr;
}

RenderCommand::RenderCommand() : mRenderTarget(nullptr), mIndex(&zero) {}

RenderCommand::~RenderCommand()
{
  for (auto& cmd : mCmds)
  {
    cmd.Wait();
  }
}

RenderCommand& RenderCommand::operator=(RenderCommand&& other)
{
  mRenderTarget = other.mRenderTarget;
  mCmds = std::move(other.mCmds);
  mIndex = other.mIndex;
  mDrawables = std::move(other.mDrawables);

  other.mRenderTarget = nullptr;
  other.mIndex = nullptr;

  return *this;
}

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const vk::UniqueFramebuffer& frameBuffer,
                             RenderTarget::DrawableList drawables)
    : mRenderTarget(&renderTarget), mIndex(&zero), mDrawables(drawables), mView(1.0f)
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

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const std::vector<vk::UniqueFramebuffer>& frameBuffers,
                             const uint32_t& index,
                             RenderTarget::DrawableList drawables)
    : mRenderTarget(&renderTarget), mIndex(&index), mDrawables(drawables), mView(1.0f)
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

RenderCommand& RenderCommand::Submit(const glm::mat4& view)
{
  mView = view;

  if (mRenderTarget)
  {
    mRenderTarget->Submit(*this);
  }

  return *this;
}

void RenderCommand::Wait()
{
  assert(mIndex);
  assert(*mIndex < mCmds.size());
  mCmds[*mIndex].Wait();
}

void RenderCommand::Render(const std::initializer_list<vk::Semaphore>& waitSemaphores,
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

RenderCommand::operator bool() const
{
  return !mCmds.empty();
}

}  // namespace Renderer
}  // namespace Vortex2D
