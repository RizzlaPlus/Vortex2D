//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

#include <Vortex2D/Renderer/Pipeline.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Pressure::Pressure(Renderer::Device& device,
                   float dt,
                   const glm::ivec2& size,
                   LinearSolver::Data& data,
                   Velocity& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::GenericBuffer& valid)
    : mData(data)
    , mBuildMatrix(device, size, SPIRV::BuildMatrix_comp)
    , mBuildMatrixBound(mBuildMatrix.Bind({data.Diagonal, data.Lower, liquidPhi, solidPhi}))
    , mBuildDiv(device, size, SPIRV::BuildDiv_comp)
    , mBuildDivBound(mBuildDiv.Bind({data.B, data.Diagonal, liquidPhi, solidPhi, velocity}))
    , mProject(device, size, SPIRV::Project_comp)
    , mProjectBound(
          mProject.Bind({data.X, liquidPhi, solidPhi, velocity, velocity.Output(), valid}))
    , mBuildEquationCmd(device, false)
    , mProjectCmd(device, false)
{
  mBuildEquationCmd.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("Build equations", {0.02f, 0.68f, 0.84f, 1.0f});
    mBuildMatrixBound.PushConstant(command, dt);
    mBuildMatrixBound.Record(command);
    data.Diagonal.Barrier(
        command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    data.Lower.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    mBuildDivBound.Record(command);
    data.B.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    command.DebugMarkerEnd();
  });

  mProjectCmd.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("Pressure", {0.45f, 0.47f, 0.75f, 1.0f});
    valid.Clear(command);
    mProjectBound.PushConstant(command, dt);
    mProjectBound.Record(command);
    velocity.CopyBack(command);
    command.DebugMarkerEnd();
  });
}

Renderer::Work::Bound Pressure::BindMatrixBuild(const glm::ivec2& size,
                                                Renderer::GenericBuffer& diagonal,
                                                Renderer::GenericBuffer& lower,
                                                Renderer::Texture& liquidPhi,
                                                Renderer::Texture& solidPhi)
{
  return mBuildMatrix.Bind(size, {diagonal, lower, liquidPhi, solidPhi});
}

void Pressure::BuildLinearEquation()
{
  mBuildEquationCmd.Submit();
}

void Pressure::ApplyPressure()
{
  mProjectCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex2D
