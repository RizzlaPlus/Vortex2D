//
//  Reflection.h
//  Vortex2D
//

#ifndef Vortex2D_Reflection_h
#define Vortex2D_Reflection_h

#include <Vortex2D/Renderer/Common.h>
#include <map>

namespace Vortex2D
{
namespace SPIRV
{
class Reflection
{
public:
  using DescriptorTypesMap = std::map<unsigned, vk::DescriptorType>;

  VORTEX2D_API Reflection(const Renderer::SpirvBinary& spirv);

  VORTEX2D_API DescriptorTypesMap GetDescriptorTypesMap() const;
  VORTEX2D_API unsigned GetPushConstantsSize() const;

  VORTEX2D_API Renderer::ShaderStage GetShaderStage() const;

private:
  DescriptorTypesMap mDescriptorTypes;
  unsigned mPushConstantSize;
  Renderer::ShaderStage mStageFlag;
};

}  // namespace SPIRV
}  // namespace Vortex2D

#endif
