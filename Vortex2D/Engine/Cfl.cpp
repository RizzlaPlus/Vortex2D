//
//  Cfl.cpp
//  Vortex2D
//

#include "Cfl.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Cfl::Cfl(Renderer::Device& device, const glm::ivec2& size, Velocity& velocity)
    : mDevice(device)
    , mSize(size)
    , mVelocity(velocity)
    , mVelocityMaxWork(device, size, SPIRV::VelocityMax_comp)
    , mVelocityMax(device, size.x * size.y)
    , mCfl(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mVelocityMaxCmd(mDevice, true)
    , mReduceVelocityMax(device, size)
{
  mVelocityMaxBound = mVelocityMaxWork.Bind({mVelocity, mVelocityMax});
  mReduceVelocityMaxBound = mReduceVelocityMax.Bind(mVelocityMax, mCfl);
  mVelocityMaxCmd.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("CFL", {0.65f, 0.97f, 0.78f, 1.0f});

    mVelocityMaxBound.Record(command);
    mReduceVelocityMaxBound.Record(command);

    command.DebugMarkerEnd();
  });
}

void Cfl::Compute()
{
  mVelocityMaxCmd.Submit();
}

float Cfl::Get()
{
  mVelocityMaxCmd.Wait();

  float cfl;
  Renderer::CopyTo(mCfl, cfl);

  return 1.0f / (cfl * mSize.x);
}

}  // namespace Fluid
}  // namespace Vortex2D
