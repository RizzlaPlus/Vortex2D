//
//  Scale.cpp
//  Vortex2D
//

#include "Scale.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

Scale::Scale(const Renderer::Device& device, int numFineGrid)
    : mDevice(device)
    , mNumFineGrid(numFineGrid)
    , mDownSampleWork(device, Renderer::ComputeSize::Default2D(), SPIRV::PhiScale_comp)
    , mDownSamplePositiveWork(device, Renderer::ComputeSize::Default2D(), SPIRV::PhiScalePositive_comp)
{

}

void Scale::DownSampleBind(std::size_t level, const glm::ivec2& fineSize,
                           Renderer::Texture& fine, 
                           Renderer::Texture& coarse)
{
  if (mDownSampleBound.size() < level + 1)
  {
    mDownSampleBound.resize(level + 1);
    mDownSampleBuffer.resize(level + 1);
  }

  if (level < mNumFineGrid)
  {
    mDownSampleBound[level] = mDownSamplePositiveWork.Bind(fineSize, {fine, coarse});
  }
  else
  {
    mDownSampleBound[level] = mDownSampleWork.Bind(fineSize, {fine, coarse});
  }

  mDownSampleBuffer[level] = &fine;
}

void Scale::DownSample(vk::CommandBuffer commandBuffer, std::size_t level)
{
    assert(level < mDownSampleBound.size());

    mDownSampleBound[level].Record(commandBuffer);
    mDownSampleBuffer[level]->Barrier(commandBuffer,
                                      vk::ImageLayout::eGeneral,
                                      vk::AccessFlagBits::eShaderWrite,
                                      vk::ImageLayout::eGeneral,
                                      vk::AccessFlagBits::eShaderRead);
}

}}
