//
//  Pipeline.cpp
//  Vortex2D
//

#include "Pipeline.h"
#include <Vortex2D/Renderer/Device.h>
#include <algorithm>

namespace Vortex2D
{
namespace Renderer
{
GraphicsPipelineDescriptor::GraphicsPipelineDescriptor()
{
  InputAssembly =
      vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

  RasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
                          .setLineWidth(1.0f)
                          .setCullMode(vk::CullModeFlagBits::eNone)
                          .setFrontFace(vk::FrontFace::eCounterClockwise)
                          .setPolygonMode(vk::PolygonMode::eFill);

  // TODO multisample as parameter
  MultisampleInfo = vk::PipelineMultisampleStateCreateInfo()
                        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                        .setMinSampleShading(1.0f);
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Shader(vk::ShaderModule shader,
                                                               ShaderStage shaderStage)
{
  auto shaderStageInfo =
      vk::PipelineShaderStageCreateInfo().setModule(shader).setPName("main").setStage(
          ConvertShaderStage(shaderStage));

  ShaderStages.push_back(shaderStageInfo);

  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::VertexAttribute(uint32_t location,
                                                                        uint32_t binding,
                                                                        Format format,
                                                                        uint32_t offset)
{
  VertexAttributeDescriptions.push_back({location, binding, ConvertFormat(format), offset});
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::VertexBinding(uint32_t binding,
                                                                      uint32_t stride,
                                                                      vk::VertexInputRate inputRate)
{
  VertexBindingDescriptions.push_back({binding, stride, inputRate});
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Topology(vk::PrimitiveTopology topology)
{
  InputAssembly.setTopology(topology);
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Layout(vk::PipelineLayout pipelineLayout)
{
  PipelineLayout = pipelineLayout;
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::DynamicState(vk::DynamicState dynamicState)
{
  DynamicStates.push_back(dynamicState);
  return *this;
}

bool operator==(const GraphicsPipelineDescriptor& left, const GraphicsPipelineDescriptor& right)
{
  return left.DynamicStates == right.DynamicStates && left.InputAssembly == right.InputAssembly &&
         left.PipelineLayout == right.PipelineLayout &&
         left.MultisampleInfo == right.MultisampleInfo &&
         left.RasterizationInfo == right.RasterizationInfo &&
         left.VertexBindingDescriptions == right.VertexBindingDescriptions &&
         left.VertexAttributeDescriptions == right.VertexAttributeDescriptions &&
         left.ShaderStages == right.ShaderStages;
}

SpecConstInfo::SpecConstInfo() {}

bool operator==(const SpecConstInfo& left, const SpecConstInfo& right)
{
  return left.data == right.data && left.mapEntries == right.mapEntries;
}

}  // namespace Renderer
}  // namespace Vortex2D
