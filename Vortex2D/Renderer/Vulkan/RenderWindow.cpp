//
//  RenderWindow.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/RenderWindow.h>

#include <Vortex2D/Renderer/Vulkan/RenderPass.h>

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Drawable.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
void TextureBarrier(vk::Image image,
                    vk::CommandBuffer commandBuffer,
                    vk::ImageLayout oldLayout,
                    vk::AccessFlags srcMask,
                    vk::ImageLayout newLayout,
                    vk::AccessFlags dstMask);

struct SwapChainSupportDetails
{
  SwapChainSupportDetails(vk::PhysicalDevice device, vk::SurfaceKHR surface)
      : capabilities(device.getSurfaceCapabilitiesKHR(surface))
      , formats(device.getSurfaceFormatsKHR(surface))
      , presentModes(device.getSurfacePresentModesKHR(surface))
  {
  }

  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;

  bool IsValid() const { return !formats.empty() && !presentModes.empty(); }
};

struct RenderWindow::Impl
{
  Impl(RenderWindow& self, Device& device, vk::SurfaceKHR surface)
      : mSelf(self), mDevice(static_cast<VulkanDevice&>(device)), mIndex(0), mFrameIndex(0)
  {
    // get swap chain support details
    SwapChainSupportDetails details(mDevice.GetPhysicalDevice(), surface);
    if (!details.IsValid())
    {
      throw std::runtime_error("Swap chain support invalid");
    }

    // TODO verify this value is valid
    uint32_t numFramebuffers = details.capabilities.minImageCount + 1;

    // TODO choose given the details
    auto format = Format::B8G8R8A8Unorm;

    // TODO find if better mode is available
    auto mode = vk::PresentModeKHR::eFifo;

    // create swap chain
    auto swapChainInfo = vk::SwapchainCreateInfoKHR()
                             .setSurface(surface)
                             .setImageFormat(ConvertFormat(format))
                             .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                             .setMinImageCount(numFramebuffers)
                             .setImageExtent({mSelf.Width, mSelf.Height})
                             .setImageArrayLayers(1)
                             .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment |
                                            vk::ImageUsageFlagBits::eTransferDst)
                             .setImageSharingMode(vk::SharingMode::eExclusive)
                             .setPreTransform(details.capabilities.currentTransform)
                             .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                             .setPresentMode(mode)
                             .setClipped(true);

    mSwapChain = mDevice.Handle().createSwapchainKHRUnique(swapChainInfo);

    // create swap chain image views
    std::vector<vk::Image> swapChainImages = mDevice.Handle().getSwapchainImagesKHR(*mSwapChain);
    for (const auto& image : swapChainImages)
    {
      auto imageViewInfo = vk::ImageViewCreateInfo()
                               .setImage(image)
                               .setViewType(vk::ImageViewType::e2D)
                               .setFormat(ConvertFormat(format))
                               .setComponents({vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity})
                               .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

      mSwapChainImageViews.push_back(mDevice.Handle().createImageViewUnique(imageViewInfo));

      mDevice.Execute([&](CommandEncoder& command) {
        TextureBarrier(image,
                       command.Handle(),
                       vk::ImageLayout::eUndefined,
                       vk::AccessFlags{},
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eTransferWrite);

        auto clearValue = vk::ClearColorValue().setFloat32({{0.0f, 0.0f, 0.0f, 0.0f}});

        command.Handle().clearColorImage(
            image,
            vk::ImageLayout::eGeneral,
            clearValue,
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        TextureBarrier(image,
                       command.Handle(),
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eTransferWrite,
                       vk::ImageLayout::ePresentSrcKHR,
                       vk::AccessFlags{});
      });
    }

    // Create render pass
    mSelf.RenderPass =
        RenderpassBuilder()
            .Attachement(format)
            .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
            .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
            .AttachementInitialLayout(vk::ImageLayout::ePresentSrcKHR)
            .AttachementFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .Subpass(vk::PipelineBindPoint::eGraphics)
            .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
            .Dependency(VK_SUBPASS_EXTERNAL, 0)
            .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .Create(mDevice.Handle());

    // Create framebuffers
    for (const auto& imageView : mSwapChainImageViews)
    {
      vk::ImageView attachments[] = {*imageView};
      auto framebufferInfo = vk::FramebufferCreateInfo()
                                 .setRenderPass(*mSelf.RenderPass)
                                 .setAttachmentCount(1)
                                 .setPAttachments(attachments)
                                 .setWidth(mSelf.Width)
                                 .setHeight(mSelf.Height)
                                 .setLayers(1);

      mFrameBuffers.push_back(mDevice.Handle().createFramebufferUnique(framebufferInfo));

      // Create semaphores
      mImageAvailableSemaphores.push_back(mDevice.Handle().createSemaphoreUnique({}));
      mRenderFinishedSemaphores.push_back(mDevice.Handle().createSemaphoreUnique({}));
    }
  }

  ~Impl() {}

  RenderCommand Record(RenderTarget::DrawableList drawables, ColorBlendState blendState)
  {
    RenderState state(mSelf, blendState);
    return RenderCommand(mDevice, mSelf, state, mFrameBuffers, mIndex, drawables);
  }

  void Submit(RenderCommand& renderCommand) { mRenderCommands.emplace_back(renderCommand); }

  void Display()
  {
    if (mRenderCommands.empty())
      return;  // nothing to draw

    auto result = mDevice.Handle().acquireNextImageKHR(
        *mSwapChain, UINT64_MAX, *mImageAvailableSemaphores[mFrameIndex], nullptr);
    if (result.result == vk::Result::eSuccess)
    {
      mIndex = result.value;
    }
    else
    {
      throw std::runtime_error("Acquire error " + vk::to_string(result.result));
    }

    if (mRenderCommands.size() == 1)
    {
      mRenderCommands[0].get().Render({*mImageAvailableSemaphores[mFrameIndex]},
                                      {*mRenderFinishedSemaphores[mFrameIndex]});
    }
    else
    {
      mRenderCommands.front().get().Render({*mImageAvailableSemaphores[mFrameIndex]});
      for (std::size_t i = 1; i < mRenderCommands.size() - 1; i++)
      {
        mRenderCommands[i].get().Render();
      }
      mRenderCommands.back().get().Render({}, {*mRenderFinishedSemaphores[mFrameIndex]});
    }

    vk::SwapchainKHR swapChain[] = {*mSwapChain};
    vk::Semaphore waitSemaphores[] = {*mRenderFinishedSemaphores[mFrameIndex]};

    auto presentInfo = vk::PresentInfoKHR()
                           .setSwapchainCount(1)
                           .setPSwapchains(swapChain)
                           .setPImageIndices(&mIndex)
                           .setPWaitSemaphores(waitSemaphores)
                           .setWaitSemaphoreCount(1);

    mDevice.Queue().presentKHR(presentInfo);
    mRenderCommands.clear();

    mFrameIndex = (mFrameIndex + 1) % mFrameBuffers.size();
  }

  RenderWindow& mSelf;
  VulkanDevice& mDevice;
  vk::UniqueSwapchainKHR mSwapChain;
  std::vector<vk::UniqueImageView> mSwapChainImageViews;
  std::vector<vk::UniqueFramebuffer> mFrameBuffers;
  std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
  std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
  std::vector<std::reference_wrapper<RenderCommand>> mRenderCommands;
  uint32_t mIndex;
  uint32_t mFrameIndex;
};

RenderWindow::RenderWindow(Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
    : RenderTarget(width, height), mImpl(std::make_unique<Impl>(*this, device, surface))
{
}

RenderWindow::~RenderWindow() {}

RenderCommand RenderWindow::Record(DrawableList drawables, ColorBlendState blendState)
{
  return mImpl->Record(drawables, blendState);
}
void RenderWindow::Submit(RenderCommand& renderCommand)
{
  mImpl->Submit(renderCommand);
}

void RenderWindow::Display()
{
  mImpl->Display();
}

}  // namespace Renderer
}  // namespace Vortex2D
