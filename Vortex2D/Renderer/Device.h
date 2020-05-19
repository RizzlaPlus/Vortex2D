//
//  Device.h
//  Vortex2D
//

#ifndef Vortex2d_Device_h
#define Vortex2d_Device_h

#include <Vortex2D/Renderer/BindGroup.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Vulkan/Instance.h>
#include <map>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Encapsulation around the vulkan device. Allows to create command
 * buffers, layout, bindings, memory and shaders.
 */
class Device
{
public:
  VORTEX2D_API Device() = default;
  VORTEX2D_API virtual ~Device() = default;

  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  VORTEX2D_API virtual void WaitIdle() = 0;

  VORTEX2D_API virtual bool HasTimer() const = 0;

  VORTEX2D_API virtual void Execute(CommandBuffer::CommandFn commandFn) const = 0;

  VORTEX2D_API virtual Handle::ShaderModule CreateShaderModule(const SpirvBinary& spirv) = 0;

  /**
   * @brief Create, cache and return a descriptor layout given the pipeline
   * layout
   * @param layout pipeline layout
   * @return cached descriptor set layout
   */
  VORTEX2D_API virtual Handle::BindGroupLayout CreateBindGroupLayout(
      const SPIRV::ShaderLayouts& layout) = 0;

  /**
   * @brief create, cache and return a vulkan pipeline layout given the layout
   * @param layout pipeline layout
   * @return vulkan pipeline layout
   */
  VORTEX2D_API virtual Handle::PipelineLayout CreatePipelineLayout(
      const SPIRV::ShaderLayouts& layout) = 0;

  VORTEX2D_API virtual BindGroup CreateBindGroup(
      const Handle::BindGroupLayout& bindGroupLayout,
      const SPIRV::ShaderLayouts& layout,
      const std::vector<BindingInput>& bindingInputs) = 0;

  /**
   * @brief Create a graphics pipeline
   * @param builder
   * @param renderState
   * @return
   */
  VORTEX2D_API virtual Handle::Pipeline CreateGraphicsPipeline(
      const GraphicsPipelineDescriptor& builder,
      const RenderState& renderState) = 0;

  /**
   * @brief Create a compute pipeline
   * @param shader
   * @param layout
   * @param specConstInfo
   */
  VORTEX2D_API virtual Handle::Pipeline CreateComputePipeline(Handle::ShaderModule shader,
                                                              Handle::PipelineLayout layout,
                                                              SpecConstInfo specConstInfo = {}) = 0;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
