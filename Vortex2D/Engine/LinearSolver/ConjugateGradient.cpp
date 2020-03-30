//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"

#include <Vortex2D/Engine/Rigidbody.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
ConjugateGradient::ConjugateGradient(Renderer::Device& device,
                                     const glm::ivec2& size,
                                     Preconditioner& preconditioner)
    : mDevice(device)
    , mPreconditioner(preconditioner)
    , r(device, size.x * size.y)
    , s(device, size.x * size.y)
    , z(device, size.x * size.y)
    , inner(device, size.x * size.y)
    , alpha(device, 1)
    , beta(device, 1)
    , rho(device, 1)
    , rho_new(device, 1)
    , sigma(device, 1)
    , error(device)
    , localError(device, 1, Renderer::MemoryUsage::GpuToCpu)
    , matrixMultiply(device, size, SPIRV::MultiplyMatrix_comp)
    , scalarDivision(device, glm::ivec2(1), SPIRV::Divide_comp)
    , scalarMultiply(device, size, SPIRV::Multiply_comp)
    , multiplyAdd(device, size, SPIRV::MultiplyAdd_comp)
    , multiplySub(device, size, SPIRV::MultiplySub_comp)
    , reduceSum(device, size)
    , reduceMax(device, size)
    , reduceMaxBound(reduceMax.Bind(r, error))
    , reduceSumRhoBound(reduceSum.Bind(inner, rho))
    , reduceSumSigmaBound(reduceSum.Bind(inner, sigma))
    , reduceSumRhoNewBound(reduceSum.Bind(inner, rho_new))
    , multiplySBound(scalarMultiply.Bind({z, s, inner}))
    , multiplyZBound(scalarMultiply.Bind({z, r, inner}))
    , divideRhoBound(scalarDivision.Bind({rho, sigma, alpha}))
    , divideRhoNewBound(scalarDivision.Bind({rho_new, rho, beta}))
    , multiplySubRBound(multiplySub.Bind({r, z, alpha, r}))
    , multiplyAddZBound(multiplyAdd.Bind({z, s, beta, s}))
    , mSolveInit(device, false)
    , mSolve(device, false)
    , mErrorRead(device)
{
  mErrorRead.Record(
      [&](Renderer::CommandEncoder& command) { localError.CopyFrom(command, error); });
}

ConjugateGradient::~ConjugateGradient() {}

void ConjugateGradient::Bind(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
  mPreconditioner.Bind(d, l, r, z);

  matrixMultiplyBound = matrixMultiply.Bind({d, l, s, z});
  multiplyAddPBound = multiplyAdd.Bind({pressure, s, alpha, pressure});

  mSolveInit.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("PCG Init", {0.63f, 0.04f, 0.66f, 1.0f});

    // r = b
    r.CopyFrom(command, b);

    // calculate error
    reduceMaxBound.Record(command);

    // p = 0
    pressure.Clear(command);

    // z = M^-1 r
    z.Clear(command);
    mPreconditioner.Record(command);
    z.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // s = z
    s.CopyFrom(command, z);

    // rho = zTr
    multiplyZBound.Record(command);
    inner.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    reduceSumRhoBound.Record(command);
    z.Clear(command);

    command.DebugMarkerEnd();
  });

  mSolve.Record([&](Renderer::CommandEncoder& command) {
    command.DebugMarkerBegin("PCG Step", {0.51f, 0.90f, 0.72f, 1.0f});

    // z = As
    matrixMultiplyBound.Record(command);
    z.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // sigma = zTs
    multiplySBound.Record(command);
    inner.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    reduceSumSigmaBound.Record(command);

    // alpha = rho / sigma
    divideRhoBound.Record(command);
    alpha.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // p = p + alpha * s
    multiplyAddPBound.Record(command);
    pressure.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // r = r - alpha * z
    multiplySubRBound.Record(command);
    r.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // calculate max error
    reduceMaxBound.Record(command);

    // z = M^-1 r
    z.Clear(command);
    mPreconditioner.Record(command);
    z.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // rho_new = zTr
    multiplyZBound.Record(command);
    inner.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    reduceSumRhoNewBound.Record(command);

    // beta = rho_new / rho
    divideRhoNewBound.Record(command);
    beta.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // s = z + beta * s
    multiplyAddZBound.Record(command);
    z.Clear(command);
    s.Barrier(command, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    // rho = rho_new
    rho.CopyFrom(command, rho_new);

    command.DebugMarkerEnd();
  });
}

void ConjugateGradient::BindRigidbody(float delta, Renderer::GenericBuffer& d, RigidBody& rigidBody)
{
  rigidBody.BindPressure(delta, d, s, z);
}

void ConjugateGradient::Solve(Parameters& params, const std::vector<RigidBody*>& rigidbodies)
{
  params.Reset();

  mSolveInit.Submit();

  if (params.Type == Parameters::SolverType::Iterative)
  {
    mErrorRead.Submit().Wait();

    Renderer::CopyTo(localError, params.OutError);
    if (params.OutError <= params.ErrorTolerance)
    {
      return;
    }

    mErrorRead.Submit();
  }

  auto initialError = params.OutError;
  for (unsigned i = 0; !params.IsFinished(initialError); params.OutIterations = ++i)
  {
    for (auto& rigidbody : rigidbodies)
    {
      rigidbody->Pressure();
    }

    mSolve.Submit();

    if (params.Type == Parameters::SolverType::Iterative)
    {
      mErrorRead.Wait();
      Renderer::CopyTo(localError, params.OutError);
      mErrorRead.Submit();
    }
  }
}

float ConjugateGradient::GetError()
{
  mErrorRead.Submit().Wait();

  float error;
  Renderer::CopyTo(localError, error);
  return error;
}

}  // namespace Fluid
}  // namespace Vortex2D
