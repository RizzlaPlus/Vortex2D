//
//  Device.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Device_h
#define Vortex2d_Vulkan_Device_h

#include <Vortex2D/Renderer/BindGroup.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <map>

#include "Instance.h"

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief A vulkan dynamic dispatcher that checks if the function is not null.
 */
struct DynamicDispatcher
{
  void vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
  void vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const;

  PFN_vkCmdDebugMarkerBeginEXT mVkCmdDebugMarkerBeginEXT = nullptr;
  PFN_vkCmdDebugMarkerEndEXT mVkCmdDebugMarkerEndEXT = nullptr;
};

class VulkanDevice : public Device
{
public:
  VORTEX2D_API VulkanDevice(const Instance& instance, bool validation = true);
  VORTEX2D_API VulkanDevice(const Instance& instance,
                            vk::SurfaceKHR surface,
                            bool validation = true);
  VORTEX2D_API VulkanDevice(const Instance& instance,
                            int familyIndex,
                            bool surface,
                            bool validation);

  VORTEX2D_API ~VulkanDevice();

  // Implementation of Device interface
  bool HasTimer() const override;

  void WaitIdle() override;

  void Execute(CommandBuffer::CommandFn commandFn) const override;

  vk::ShaderModule CreateShaderModule(const SpirvBinary& spirv) override;

  BindGroupLayout CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout) override;

  vk::PipelineLayout CreatePipelineLayout(const SPIRV::ShaderLayouts& layout) override;

  BindGroup CreateBindGroup(const BindGroupLayout& bindGroupLayout,
                            const SPIRV::ShaderLayouts& layout,
                            const std::vector<BindingInput>& bindingInputs) override;

  vk::Pipeline CreateGraphicsPipeline(const GraphicsPipelineDescriptor& builder,
                                      const RenderState& renderState) override;

  vk::Pipeline CreateComputePipeline(vk::ShaderModule shader,
                                     vk::PipelineLayout layout,
                                     SpecConstInfo specConstInfo = {}) override;

  CommandEncoder CreateCommandEncoder() override;

  // Vulkan specific functions
  VmaAllocator Allocator() const;

  VORTEX2D_API vk::Device Handle() const;

  VORTEX2D_API const DynamicDispatcher& Loader() const;

  VORTEX2D_API vk::Queue Queue() const;

  VORTEX2D_API vk::PhysicalDevice GetPhysicalDevice() const;

  int GetFamilyIndex() const;

private:
  void CreateDescriptorPool(int size = 512);

  vk::PhysicalDevice mPhysicalDevice;
  DynamicDispatcher mLoader;
  int mFamilyIndex;
  vk::UniqueDevice mDevice;
  vk::Queue mQueue;
  vk::UniqueCommandPool mCommandPool;
  vk::UniqueDescriptorPool mDescriptorPool;
  VmaAllocator mAllocator;
  mutable std::unique_ptr<CommandBuffer> mCommandBuffer;

  std::map<const uint32_t*, vk::UniqueShaderModule> mShaders;
  std::vector<std::tuple<SPIRV::ShaderLayouts, vk::UniqueDescriptorSetLayout>>
      mDescriptorSetLayouts;
  std::vector<std::tuple<SPIRV::ShaderLayouts, vk::UniquePipelineLayout>> mPipelineLayouts;

  struct GraphicsPipelineCache
  {
    RenderState State;
    GraphicsPipelineDescriptor Graphics;
    vk::UniquePipeline Pipeline;
  };

  struct ComputePipelineCache
  {
    vk::ShaderModule Shader;
    vk::PipelineLayout Layout;
    SpecConstInfo SpecConst;
    vk::UniquePipeline Pipeline;
  };

  std::vector<GraphicsPipelineCache> mGraphicsPipelines;
  std::vector<ComputePipelineCache> mComputePipelines;
  vk::UniquePipelineCache mPipelineCache;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
