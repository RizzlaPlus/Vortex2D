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

/**
 * @brief Encapsulation around the vulkan device. Allows to create command
 * buffers, layout, bindings, memory and shaders.
 */
class Device
{
public:
  VORTEX2D_API Device(const Instance& instance, bool validation = true);
  VORTEX2D_API Device(const Instance& instance, vk::SurfaceKHR surface, bool validation = true);
  VORTEX2D_API Device(const Instance& instance, int familyIndex, bool surface, bool validation);
  VORTEX2D_API ~Device();

  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  // Vulkan handles and helpers
  VORTEX2D_API vk::Device Handle() const;
  VORTEX2D_API vk::Queue Queue() const;
  VORTEX2D_API const DynamicDispatcher& Loader() const;
  VORTEX2D_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX2D_API int GetFamilyIndex() const;

  // Command buffer functions
  VORTEX2D_API vk::CommandBuffer CreateCommandBuffer() const;
  VORTEX2D_API void FreeCommandBuffer(vk::CommandBuffer commandBuffer) const;
  VORTEX2D_API void Execute(CommandBuffer::CommandFn commandFn) const;

  // Memory allocator
  VORTEX2D_API VmaAllocator Allocator() const;
  VORTEX2D_API vk::ShaderModule GetShaderModule(const SpirvBinary& spirv) const;

  // Device Interface

  /**
   * @brief Create, cache and return a descriptor layout given the pipeline
   * layout
   * @param layout pipeline layout
   * @return cached descriptor set layout
   */
  VORTEX2D_API BindGroupLayout CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout);

  VORTEX2D_API vk::PipelineLayout CreatePipelineLayout(const SPIRV::ShaderLayouts& layout);

  /**
   * @brief create, cache and return a vulkan pipeline layout given the layout
   * @param layout pipeline layout
   * @return vulkan pipeline layout
   */
  VORTEX2D_API BindGroup CreateBindGroup(const BindGroupLayout& bindGroupLayout,
                                         const SPIRV::ShaderLayouts& layout,
                                         const std::vector<BindingInput>& bindingInputs);

  /**
   * @brief Create a graphics pipeline
   * @param builder
   * @param renderState
   * @return
   */
  VORTEX2D_API vk::Pipeline CreateGraphicsPipeline(const GraphicsPipelineDescriptor& builder,
                                                   const RenderState& renderState);

  /**
   * @brief Create a compute pipeline
   * @param shader
   * @param layout
   * @param specConstInfo
   */
  VORTEX2D_API vk::Pipeline CreateComputePipeline(vk::ShaderModule shader,
                                                  vk::PipelineLayout layout,
                                                  SpecConstInfo specConstInfo = {});

private:
  /**
   * @brief Create or re-create the descriptor pool, will render invalid
   * existing descriptor sets
   * @param size size of the pool
   */
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
  mutable std::map<const uint32_t*, vk::UniqueShaderModule> mShaders;

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
  vk::UniquePipelineCache mCache;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
