//
//  RenderTexture.cpp
//  Vortex
//

#include <Vortex2D/Renderer/RenderTexture.h>

#include <Vortex2D/Renderer/Vulkan/RenderPass.h>

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Drawable.h>

namespace Vortex2D
{
namespace Renderer
{
struct RenderTexture::Impl
{
  Impl(RenderTexture& self, Device& device, Format format) : mSelf(self), mDevice(device)
  {
    // Create render pass
    mSelf.RenderPass =
        RenderpassBuilder()
            .Attachement(format)
            .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
            .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
            // TODO should they both be general?
            .AttachementInitialLayout(vk::ImageLayout::eGeneral)
            .AttachementFinalLayout(vk::ImageLayout::eGeneral)
            .Subpass(vk::PipelineBindPoint::eGraphics)
            .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
            .Dependency(VK_SUBPASS_EXTERNAL, 0)
            .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
            .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .Create(device.Handle());

    // Create framebuffer
    vk::ImageView attachments[] = {mSelf.GetView()};

    auto framebufferInfo = vk::FramebufferCreateInfo()
                               .setWidth(mSelf.Width)
                               .setHeight(mSelf.Height)
                               .setRenderPass(*mSelf.RenderPass)
                               .setAttachmentCount(1)
                               .setPAttachments(attachments)
                               .setLayers(1);

    mFramebuffer = device.Handle().createFramebufferUnique(framebufferInfo);
  }

  RenderCommand Record(DrawableList drawables, ColorBlendState blendState)
  {
    RenderState state(mSelf, blendState);
    return RenderCommand(mDevice, mSelf, state, mFramebuffer, drawables);
  }

  void Submit(RenderCommand& renderCommand) { renderCommand.Render(); }

  RenderTexture& mSelf;
  Device& mDevice;
  vk::UniqueFramebuffer mFramebuffer;
};

RenderTexture::RenderTexture(Device& device, uint32_t width, uint32_t height, Format format)
    : RenderTarget(width, height)
    , Texture(device, width, height, format)
    , mImpl(std::make_unique<Impl>(*this, device, format))
{
}

RenderTexture::RenderTexture(RenderTexture&& other)
    : RenderTarget(std::move(other)), Texture(std::move(other)), mImpl(std::move(other.mImpl))
{
}

RenderTexture::~RenderTexture() {}

RenderCommand RenderTexture::Record(DrawableList drawables, ColorBlendState blendState)
{
  return mImpl->Record(drawables, blendState);
}

void RenderTexture::Submit(RenderCommand& renderCommand)
{
  mImpl->Submit(renderCommand);
}

}  // namespace Renderer
}  // namespace Vortex2D
