//
//  IncompletePoisson.cpp
//  Vortex2D
//

#include "IncompletePoisson.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
IncompletePoisson::IncompletePoisson(Renderer::Device& device, const glm::ivec2& size)
    : mIncompletePoisson(device, size, SPIRV::IncompletePoisson_comp)
{
}

IncompletePoisson::~IncompletePoisson() {}

void IncompletePoisson::Bind(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
  mIncompletePoissonBound = mIncompletePoisson.Bind({d, l, b, pressure});
}

void IncompletePoisson::Record(Renderer::CommandEncoder& command)
{
  mIncompletePoissonBound.Record(command);
}

}  // namespace Fluid
}  // namespace Vortex2D
