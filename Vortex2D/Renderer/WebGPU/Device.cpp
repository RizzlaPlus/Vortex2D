//
//  Device.cpp
//  Vortex2D
//

#include <fstream>
#include <iostream>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
WebGPUDevice::WebGPUDevice(const Instance& instance)
    : mDevice(wgpu_adapter_request_device(instance.GetAdapter(), nullptr))
{
}

WebGPUDevice::~WebGPUDevice() {}

bool WebGPUDevice::HasTimer() const
{
  return false;
}

void WebGPUDevice::WaitIdle() {}

void WebGPUDevice::Execute(CommandBuffer::CommandFn commandFn) const {}

Handle::ShaderModule WebGPUDevice::CreateShaderModule(const SpirvBinary& spirv)
{
  return {};
}

Handle::BindGroupLayout WebGPUDevice::CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout)
{
  return {};
}

Handle::PipelineLayout WebGPUDevice::CreatePipelineLayout(const SPIRV::ShaderLayouts& layout)
{
  return {};
}

BindGroup WebGPUDevice::CreateBindGroup(const Handle::BindGroupLayout& bindGroupLayout,
                                        const SPIRV::ShaderLayouts& layout,
                                        const std::vector<BindingInput>& bindingInputs)
{
  return {};
}

Handle::Pipeline WebGPUDevice::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& graphics,
                                                      const RenderState& renderState)
{
  return {};
}

Handle::Pipeline WebGPUDevice::CreateComputePipeline(Handle::ShaderModule shader,
                                                     Handle::PipelineLayout layout,
                                                     SpecConstInfo specConstInfo)
{
  return {};
}

}  // namespace Renderer
}  // namespace Vortex2D
