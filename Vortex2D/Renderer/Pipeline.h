//
//  Pipeline.h
//  Vortex2D
//

#ifndef Vortex2d_Shader_h
#define Vortex2d_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <string>
#include <vector>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief graphics pipeline which caches the pipeline per render states.
 */
class GraphicsPipelineDescriptor
{
public:
  VORTEX2D_API GraphicsPipelineDescriptor();

  /**
   * @brief Set the shader
   * @param shader the loaded shader
   * @param shaderStage shader state (vertex, fragment or compute)
   * @return *this
   */
  VORTEX2D_API GraphicsPipelineDescriptor& Shader(Handle::ShaderModule shader, ShaderStage shaderStage);

  /**
   * @brief Sets the vertex attributes
   * @param location location in the shader
   * @param binding binding in the shader
   * @param format vertex format
   * @param offset offset in the vertex
   * @return *this
   */
  VORTEX2D_API GraphicsPipelineDescriptor& VertexAttribute(uint32_t location,
                                                           uint32_t binding,
                                                           Format format,
                                                           uint32_t offset);

  /**
   * @brief Sets the vertex binding
   * @param binding binding in the shader
   * @param stride stride in bytes
   * @return *this
   */
  VORTEX2D_API GraphicsPipelineDescriptor& VertexBinding(uint32_t binding, uint32_t stride);

  VORTEX2D_API GraphicsPipelineDescriptor& Topology(PrimitiveTopology topology);
  VORTEX2D_API GraphicsPipelineDescriptor& Layout(Handle::PipelineLayout pipelineLayout);

  struct ShaderDescriptor
  {
    Handle::ShaderModule shaderModule;
    ShaderStage shaderStage;
  };

  std::vector<ShaderDescriptor> shaders;

  struct VertexAttributeDescriptor
  {
    uint32_t location;
    uint32_t binding;
    Format format;
    uint32_t offset;
  };

  std::vector<VertexAttributeDescriptor> vertexAttributes;

  struct VertexBindingDescriptor
  {
    uint32_t binding;
    uint32_t stride;
  };

  std::vector<VertexBindingDescriptor> vertexBindings;
  PrimitiveTopology primitiveTopology = PrimitiveTopology::Triangle;
  Handle::PipelineLayout pipelineLayout;
};

bool operator==(const GraphicsPipelineDescriptor::ShaderDescriptor& left,
                const GraphicsPipelineDescriptor::ShaderDescriptor& right);
bool operator==(const GraphicsPipelineDescriptor::VertexBindingDescriptor& left,
                const GraphicsPipelineDescriptor::VertexBindingDescriptor& right);
bool operator==(const GraphicsPipelineDescriptor::VertexAttributeDescriptor& left,
                const GraphicsPipelineDescriptor::VertexAttributeDescriptor& right);
bool operator==(const GraphicsPipelineDescriptor& left, const GraphicsPipelineDescriptor& right);

/**
 * @brief Defines and holds value of the specification constants for shaders
 */
struct SpecConstInfo
{
  VORTEX2D_API SpecConstInfo();

  template <typename Type>
  struct Value
  {
    uint32_t id;
    Type value;
  };

  vk::SpecializationInfo info;
  std::vector<vk::SpecializationMapEntry> mapEntries;
  std::vector<char> data;
};

bool operator==(const SpecConstInfo& left, const SpecConstInfo& right);

namespace Detail
{
inline void InsertSpecConst(SpecConstInfo& specConstInfo)
{
  specConstInfo.info.setMapEntryCount(static_cast<uint32_t>(specConstInfo.mapEntries.size()))
      .setPMapEntries(specConstInfo.mapEntries.data())
      .setDataSize(specConstInfo.data.size())
      .setPData(specConstInfo.data.data());
}

template <typename Arg, typename... Args>
inline void InsertSpecConst(SpecConstInfo& specConstInfo, Arg&& arg, Args&&... args)
{
  auto offset = static_cast<uint32_t>(specConstInfo.data.size());
  specConstInfo.data.resize(offset + sizeof(Arg));
  std::memcpy(&specConstInfo.data[offset], &arg.value, sizeof(Arg));
  specConstInfo.mapEntries.emplace_back(arg.id, offset, sizeof(Arg));

  InsertSpecConst(specConstInfo, std::forward<Args>(args)...);
}
}  // namespace Detail

/**
 * @brief Constructs a specialization constant value
 */
template <typename Type>
inline SpecConstInfo::Value<Type> SpecConstValue(uint32_t id, Type value)
{
  return SpecConstInfo::Value<Type>{id, value};
}

/**
 * @brief Constructs a @ref SpecConstInfo with given values of specialisation
 * constants.
 */
template <typename... Args>
inline SpecConstInfo SpecConst(Args&&... args)
{
  SpecConstInfo specConstInfo;
  Detail::InsertSpecConst(specConstInfo, std::forward<Args>(args)...);
  return specConstInfo;
}

}  // namespace Renderer
}  // namespace Vortex2D

#endif
