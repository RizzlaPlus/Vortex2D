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
{
  mDevice = wgpu_adapter_request_device(instance.GetAdapter(), nullptr, nullptr);
  // TODO check device is valid

  mQueue = wgpu_device_get_default_queue(mDevice);
  // TODO check queue is valid
}

WebGPUDevice::~WebGPUDevice()
{
  if (mDevice != 0)
  {
    wgpu_device_destroy(mDevice);
  }
}

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

WGPUDeviceId WebGPUDevice::Handle() const
{
  return mDevice;
}

WGPUQueueId WebGPUDevice::Queue() const
{
  return mQueue;
}

}  // namespace Renderer
}  // namespace Vortex2D
