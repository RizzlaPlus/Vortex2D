//
//  Device.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Device.h>

#include <fstream>
#include <iostream>

#include "Instance.h"
#include "Vulkan.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Vortex2D
{
namespace Renderer
{
namespace
{
int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface = nullptr)
{
  int index = -1;
  const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
  for (std::size_t i = 0; i < familyProperties.size(); i++)
  {
    const auto& property = familyProperties[i];
    if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
        (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (!surface || physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface)))
    {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index == -1)
  {
    throw std::runtime_error("Suitable physical device not found");
  }

  return index;
}

vk::DescriptorType GetDescriptorType(uint32_t bind, const SPIRV::ShaderLayouts& layout)
{
  for (auto& shaderLayout : layout)
  {
    auto it = shaderLayout.bindings.find(bind);
    if (it != shaderLayout.bindings.end())
    {
      return it->second;
    }
  }

  throw std::runtime_error("no bindings defined");
}

void Bind(vk::Device device,
          BindGroup& dstSet,
          const SPIRV::ShaderLayouts& layout,
          const std::vector<BindingInput>& bindingInputs)
{
  std::vector<vk::DescriptorBufferInfo> bufferInfo(20);
  std::vector<vk::DescriptorImageInfo> imageInfo(20);
  std::vector<vk::WriteDescriptorSet> descriptorWrites;
  std::size_t numBuffers = 0;
  std::size_t numImages = 0;

  for (std::size_t i = 0; i < bindingInputs.size(); i++)
  {
    bindingInputs[i].Input.match(
        [&](Renderer::GenericBuffer* buffer) {
          uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind
                              ? static_cast<uint32_t>(i)
                              : bindingInputs[i].Bind;

          auto descriptorType = GetDescriptorType(bind, layout);
          if (descriptorType != vk::DescriptorType::eStorageBuffer &&
              descriptorType != vk::DescriptorType::eUniformBuffer)
            throw std::runtime_error("Binding not a storage buffer");

          auto writeDescription = vk::WriteDescriptorSet()
                                      .setDstSet(*dstSet.descriptorSet)
                                      .setDstBinding(bind)
                                      .setDstArrayElement(0)
                                      .setDescriptorType(descriptorType)
                                      .setPBufferInfo(bufferInfo.data() + numBuffers);
          descriptorWrites.push_back(writeDescription);

          if (!descriptorWrites.empty() && numBuffers != bufferInfo.size() &&
              descriptorWrites.back().pBufferInfo)
          {
            descriptorWrites.back().descriptorCount++;
            bufferInfo[numBuffers++] =
                vk::DescriptorBufferInfo(buffer->Handle(), 0, buffer->Size());
          }
          else
          {
            assert(false);
          }
        },
        [&](Image image) {
          uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind
                              ? static_cast<uint32_t>(i)
                              : bindingInputs[i].Bind;

          auto descriptorType = GetDescriptorType(bind, layout);
          if (descriptorType != vk::DescriptorType::eStorageImage &&
              descriptorType != vk::DescriptorType::eCombinedImageSampler)
            throw std::runtime_error("Binding not an image");

          auto writeDescription = vk::WriteDescriptorSet()
                                      .setDstSet(*dstSet.descriptorSet)
                                      .setDstBinding(bind)
                                      .setDstArrayElement(0)
                                      .setDescriptorType(descriptorType)
                                      .setPImageInfo(imageInfo.data() + numImages);
          descriptorWrites.push_back(writeDescription);

          if (!descriptorWrites.empty() && numImages != imageInfo.size() &&
              descriptorWrites.back().pImageInfo)
          {
            descriptorWrites.back().descriptorCount++;
            imageInfo[numImages++] = vk::DescriptorImageInfo(
                image.Sampler, image.Texture->GetView(), vk::ImageLayout::eGeneral);
          }
          else
          {
            assert(false);
          }
        });
  }

  device.updateDescriptorSets(descriptorWrites, {});
}
}  // namespace

void DynamicDispatcher::vkCmdDebugMarkerBeginEXT(
    VkCommandBuffer commandBuffer,
    const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const
{
  if (mVkCmdDebugMarkerBeginEXT != nullptr)
  {
    mVkCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
  }
}

void DynamicDispatcher::vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const
{
  if (mVkCmdDebugMarkerEndEXT != nullptr)
  {
    mVkCmdDebugMarkerEndEXT(commandBuffer);
  }
}

Device::Device(const Instance& instance, bool validation)
    : Device(instance, ComputeFamilyIndex(instance.GetPhysicalDevice()), false, validation)
{
}

Device::Device(const Instance& instance, vk::SurfaceKHR surface, bool validation)
    : Device(instance, ComputeFamilyIndex(instance.GetPhysicalDevice(), surface), true, validation)
{
}

Device::Device(const Instance& instance, int familyIndex, bool surface, bool validation)
    : mPhysicalDevice(instance.GetPhysicalDevice()), mFamilyIndex(familyIndex)
{
  float queuePriority = 1.0f;
  auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setQueueCount(1)
                             .setPQueuePriorities(&queuePriority);

  std::vector<const char*> deviceExtensions;
  std::vector<const char*> validationLayers;

  if (surface)
  {
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  // make sure we request valid layers only
  auto availableLayers = mPhysicalDevice.enumerateDeviceLayerProperties();
  auto availableExtensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

  // add validation extensions and layers
  if (validation)
  {
    if (HasLayer(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME, availableLayers))
    {
      validationLayers.push_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME);
    }
    if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
    {
      deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
  }

  // create queue
  auto deviceFeatures = vk::PhysicalDeviceFeatures().setShaderStorageImageExtendedFormats(true);
  auto deviceInfo = vk::DeviceCreateInfo()
                        .setQueueCreateInfoCount(1)
                        .setPQueueCreateInfos(&deviceQueueInfo)
                        .setPEnabledFeatures(&deviceFeatures)
                        .setEnabledExtensionCount((uint32_t)deviceExtensions.size())
                        .setPpEnabledExtensionNames(deviceExtensions.data())
                        .setEnabledLayerCount((uint32_t)validationLayers.size())
                        .setPpEnabledLayerNames(validationLayers.data());

  mDevice = mPhysicalDevice.createDeviceUnique(deviceInfo);
  mQueue = mDevice->getQueue(familyIndex, 0);

  // load marker ext
  if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
  {
    mLoader.mVkCmdDebugMarkerBeginEXT =
        (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerBeginEXT");
    mLoader.mVkCmdDebugMarkerEndEXT =
        (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerEndEXT");
  }

  // create command pool
  auto commandPoolInfo = vk::CommandPoolCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  mCommandPool = mDevice->createCommandPoolUnique(commandPoolInfo);

  // create alllocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = mPhysicalDevice;
  allocatorInfo.device = *mDevice;

  if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS)
  {
    throw std::runtime_error("Error creating allocator");
  }

  // create objects depending on device
  CreateDescriptorPool();
  mCache = mDevice->createPipelineCacheUnique({});
  mCommandBuffer = std::make_unique<CommandBuffer>(*this, true);
}

Device::~Device()
{
  vmaDestroyAllocator(mAllocator);
}

void Device::CreateDescriptorPool(int size)
{
  // create descriptor pool
  std::vector<vk::DescriptorPoolSize> poolSizes;
  poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, size);
  poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, size);
  poolSizes.emplace_back(vk::DescriptorType::eStorageImage, size);
  poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, size);

  vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  descriptorPoolInfo.maxSets = 512;
  descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  mDescriptorPool = mDevice->createDescriptorPoolUnique(descriptorPoolInfo);
}

vk::Device Device::Handle() const
{
  return *mDevice;
}

vk::Queue Device::Queue() const
{
  return mQueue;
}

const DynamicDispatcher& Device::Loader() const
{
  return mLoader;
}

vk::PhysicalDevice Device::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

int Device::GetFamilyIndex() const
{
  return mFamilyIndex;
}

vk::CommandBuffer Device::CreateCommandBuffer() const
{
  auto commandBufferInfo = vk::CommandBufferAllocateInfo()
                               .setCommandBufferCount(1)
                               .setCommandPool(*mCommandPool)
                               .setLevel(vk::CommandBufferLevel::ePrimary);

  return mDevice->allocateCommandBuffers(commandBufferInfo).at(0);
}

void Device::FreeCommandBuffer(vk::CommandBuffer commandBuffer) const
{
  mDevice->freeCommandBuffers(*mCommandPool, {commandBuffer});
}

void Device::Execute(CommandBuffer::CommandFn commandFn) const
{
  (*mCommandBuffer).Record(commandFn).Submit().Wait();
}

VmaAllocator Device::Allocator() const
{
  return mAllocator;
}

vk::ShaderModule Device::GetShaderModule(const SpirvBinary& spirv) const
{
  auto it = mShaders.find(spirv.data());
  if (it != mShaders.end())
  {
    return *it->second;
  }

  if (spirv.size() == 0)
    throw std::runtime_error("Invalid SPIRV");

  auto shaderInfo = vk::ShaderModuleCreateInfo().setCodeSize(spirv.size()).setPCode(spirv.data());

  auto shaderModule = mDevice->createShaderModuleUnique(shaderInfo);
  auto shader = *shaderModule;
  mShaders[spirv.data()] = std::move(shaderModule);
  return shader;
}

BindGroupLayout Device::CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout)
{
  auto it = std::find_if(
      mDescriptorSetLayouts.begin(),
      mDescriptorSetLayouts.end(),
      [&](const auto& descriptorSetLayout) { return std::get<0>(descriptorSetLayout) == layout; });

  if (it == mDescriptorSetLayouts.end())
  {
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    for (auto& shaderLayout : layout)
    {
      for (auto& desciptorType : shaderLayout.bindings)
      {
        descriptorSetLayoutBindings.push_back(
            {desciptorType.first, desciptorType.second, 1, shaderLayout.shaderStage, nullptr});
      }
    }

    auto descriptorSetLayoutInfo =
        vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount((uint32_t)descriptorSetLayoutBindings.size())
            .setPBindings(descriptorSetLayoutBindings.data());

    auto descriptorSetLayout = mDevice->createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);
    mDescriptorSetLayouts.emplace_back(layout, std::move(descriptorSetLayout));
    return {*std::get<1>(mDescriptorSetLayouts.back())};
  }

  return {*std::get<1>(*it)};
}

vk::PipelineLayout Device::CreatePipelineLayout(const SPIRV::ShaderLayouts& layout)
{
  auto it = std::find_if(
      mPipelineLayouts.begin(), mPipelineLayouts.end(), [&](const auto& pipelineLayout) {
        return std::get<0>(pipelineLayout) == layout;
      });

  if (it == mPipelineLayouts.end())
  {
    auto bindGroupLayout = CreateBindGroupLayout(layout);
    vk::DescriptorSetLayout descriptorSetlayouts[] = {bindGroupLayout.descriptorSetLayout};
    std::vector<vk::PushConstantRange> pushConstantRanges;
    uint32_t totalPushConstantSize = 0;
    for (auto& shaderLayout : layout)
    {
      if (shaderLayout.pushConstantSize > 0)
      {
        pushConstantRanges.push_back(
            {shaderLayout.shaderStage, totalPushConstantSize, shaderLayout.pushConstantSize});
        totalPushConstantSize += shaderLayout.pushConstantSize;
      }
    }

    auto pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(descriptorSetlayouts);

    if (totalPushConstantSize > 0)
    {
      pipelineLayoutInfo.setPPushConstantRanges(pushConstantRanges.data())
          .setPushConstantRangeCount((uint32_t)pushConstantRanges.size());
    }

    mPipelineLayouts.emplace_back(layout, mDevice->createPipelineLayoutUnique(pipelineLayoutInfo));
    return *std::get<1>(mPipelineLayouts.back());
  }

  return *std::get<1>(*it);
}

BindGroup Device::CreateBindGroup(const BindGroupLayout& bindGroupLayout,
                                  const SPIRV::ShaderLayouts& layout,
                                  const std::vector<BindingInput>& bindingInputs)
{
  vk::DescriptorSetLayout descriptorSetlayouts[] = {bindGroupLayout.descriptorSetLayout};

  auto descriptorSetInfo = vk::DescriptorSetAllocateInfo()
                               .setDescriptorPool(*mDescriptorPool)
                               .setDescriptorSetCount(1)
                               .setPSetLayouts(descriptorSetlayouts);

  BindGroup bindGroup;
  bindGroup.descriptorSet =
      std::move(mDevice->allocateDescriptorSetsUnique(descriptorSetInfo).at(0));

  Bind(*mDevice, bindGroup, layout, bindingInputs);

  return bindGroup;
}

vk::Pipeline Device::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& graphics,
                                            const RenderState& renderState)
{
  auto it = std::find_if(mGraphicsPipelines.begin(),
                         mGraphicsPipelines.end(),
                         [&](const GraphicsPipelineCache& pipeline) {
                           return pipeline.Graphics == graphics && pipeline.State == renderState;
                         });

  if (it != mGraphicsPipelines.end())
  {
    return *it->Pipeline;
  }

  auto vertexInputInfo =
      vk::PipelineVertexInputStateCreateInfo()
          .setVertexBindingDescriptionCount((uint32_t)graphics.VertexBindingDescriptions.size())
          .setPVertexBindingDescriptions(graphics.VertexBindingDescriptions.data())
          .setVertexAttributeDescriptionCount((uint32_t)graphics.VertexAttributeDescriptions.size())
          .setPVertexAttributeDescriptions(graphics.VertexAttributeDescriptions.data());

  auto viewPort = vk::Viewport(0,
                               0,
                               static_cast<float>(renderState.Width),
                               static_cast<float>(renderState.Height),
                               0.0f,
                               1.0f);
  auto scissor = vk::Rect2D({0, 0}, {renderState.Width, renderState.Height});

  auto viewPortState = vk::PipelineViewportStateCreateInfo()
                           .setScissorCount(1)
                           .setPScissors(&scissor)
                           .setViewportCount(1)
                           .setPViewports(&viewPort);

  auto blendInfo = vk::PipelineColorBlendStateCreateInfo()
                       .setAttachmentCount(1)
                       .setPAttachments(&renderState.BlendState.ColorBlend)
                       .setBlendConstants(renderState.BlendState.BlendConstants);

  auto dynamicState = vk::PipelineDynamicStateCreateInfo()
                          .setPDynamicStates(graphics.DynamicStates.data())
                          .setDynamicStateCount((uint32_t)graphics.DynamicStates.size());

  auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
                          .setStageCount((uint32_t)graphics.ShaderStages.size())
                          .setPStages(graphics.ShaderStages.data())
                          .setPVertexInputState(&vertexInputInfo)
                          .setPInputAssemblyState(&graphics.InputAssembly)
                          .setPRasterizationState(&graphics.RasterizationInfo)
                          .setPMultisampleState(&graphics.MultisampleInfo)
                          .setPColorBlendState(&blendInfo)
                          .setLayout(graphics.PipelineLayout)
                          .setRenderPass(renderState.RenderPass)
                          .setPViewportState(&viewPortState)
                          .setPDynamicState(&dynamicState);

  GraphicsPipelineCache pipeline = {
      renderState, graphics, mDevice->createGraphicsPipelineUnique(*mCache, pipelineInfo)};
  mGraphicsPipelines.push_back(std::move(pipeline));
  return *mGraphicsPipelines.back().Pipeline;
}

vk::Pipeline Device::CreateComputePipeline(vk::ShaderModule shader,
                                           vk::PipelineLayout layout,
                                           SpecConstInfo specConstInfo)
{
  auto it = std::find_if(mComputePipelines.begin(),
                         mComputePipelines.end(),
                         [&](const ComputePipelineCache& pipeline) {
                           return pipeline.Shader == shader && pipeline.Layout == layout &&
                                  pipeline.SpecConst == specConstInfo;
                         });

  if (it != mComputePipelines.end())
  {
    return *it->Pipeline;
  }

  auto stageInfo = vk::PipelineShaderStageCreateInfo()
                       .setModule(shader)
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eCompute)
                       .setPSpecializationInfo(&specConstInfo.info);

  auto pipelineInfo = vk::ComputePipelineCreateInfo().setStage(stageInfo).setLayout(layout);

  mComputePipelines.push_back(
      {shader, layout, specConstInfo, mDevice->createComputePipelineUnique(*mCache, pipelineInfo)});
  return *mComputePipelines.back().Pipeline;
}

}  // namespace Renderer
}  // namespace Vortex2D
