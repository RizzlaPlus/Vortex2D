//
//  Velocity.cpp
//  Vortex2D
//

#include "Velocity.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Velocity::Velocity(Renderer::Device& device, const glm::ivec2& size)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mDevice(device)
    , mOutputVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mDVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mVelocityDiff(device, size, SPIRV::VelocityDifference_comp)
    , mVelocityDiffBound(mVelocityDiff.Bind({mDVelocity, *this, mOutputVelocity}))
    , mSaveCopyCmd(mDevice, false)
    , mVelocityDiffCmd(mDevice, false)
{
  mSaveCopyCmd.Record(
      [&](Renderer::CommandEncoder& command) { mDVelocity.CopyFrom(command, *this); });

  mVelocityDiffCmd.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("Velocity diff", {0.32f, 0.60f, 0.67f, 1.0f});
    mVelocityDiffBound.Record(command);
    mOutputVelocity.Barrier(command,
                            vk::ImageLayout::eGeneral,
                            vk::AccessFlagBits::eShaderWrite,
                            vk::ImageLayout::eGeneral,
                            vk::AccessFlagBits::eShaderRead);
    mDVelocity.CopyFrom(command, mOutputVelocity);
    command.DebugMarkerEnd();
  });
}

Renderer::Texture& Velocity::Output()
{
  return mOutputVelocity;
}

Renderer::Texture& Velocity::D()
{
  return mDVelocity;
}

void Velocity::CopyBack(Renderer::CommandEncoder& command)
{
  CopyFrom(command, mOutputVelocity);
}

void Velocity::Clear(Renderer::CommandEncoder& command)
{
  RenderTexture::Clear(command, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
}

void Velocity::SaveCopy()
{
  mSaveCopyCmd.Submit();
}

void Velocity::VelocityDiff()
{
  mVelocityDiffCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex2D
