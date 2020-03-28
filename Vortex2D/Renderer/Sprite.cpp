//
//  Sprite.cpp
//  Vortex2D
//

#include "Sprite.h"

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Renderer
{
AbstractSprite::AbstractSprite(const Device& device,
                               const SpirvBinary& fragShaderName,
                               Texture& texture)
    : mDevice(const_cast<Device&>(device))  // FIXME remove const_cast
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
    , mColourBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
{
  VertexBuffer<Vertex> localBuffer(device, 6, VMA_MEMORY_USAGE_CPU_ONLY);
  std::vector<Vertex> vertices = {{{0.0f, 0.0f}, {0.0f, 0.0f}},
                                  {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
                                  {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}},
                                  {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
                                  {{1.0f, 1.0f}, {texture.GetWidth(), texture.GetHeight()}},
                                  {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}}};

  Renderer::CopyFrom(localBuffer, vertices);
  device.Execute(
      [&](vk::CommandBuffer commandBuffer) { mVertexBuffer.CopyFrom(commandBuffer, localBuffer); });

  SPIRV::Reflection reflectionVert(SPIRV::TexturePosition_vert);
  SPIRV::Reflection reflectionFrag(fragShaderName);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mSampler = SamplerBuilder().Filter(vk::Filter::eLinear).Create(device.Handle());

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(
      bindGroupLayout, layout, {{mMVPBuffer, 0}, {*mSampler, texture, 1}, {mColourBuffer, 2}});

  vk::ShaderModule vertexShader = mDevice.CreateShaderModule(SPIRV::TexturePosition_vert);
  vk::ShaderModule fragShader = mDevice.CreateShaderModule(fragShaderName);

  mPipeline = GraphicsPipelineDescriptor()
                  .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
                  .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
                  .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos))
                  .VertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
                  .VertexBinding(0, sizeof(Vertex))
                  .Layout(mPipelineLayout);
}

AbstractSprite::AbstractSprite(AbstractSprite&& other)
    : mDevice(other.mDevice)
    , mMVPBuffer(std::move(other.mMVPBuffer))
    , mVertexBuffer(std::move(other.mVertexBuffer))
    , mColourBuffer(std::move(other.mColourBuffer))
    , mSampler(std::move(other.mSampler))
    , mPipelineLayout(std::move(other.mPipelineLayout))
    , mBindGroup(std::move(other.mBindGroup))
    , mPipeline(std::move(other.mPipeline))
{
}

AbstractSprite::~AbstractSprite() {}

void AbstractSprite::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mColourBuffer, Colour);
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
}

void AbstractSprite::Initialize(const RenderState& renderState)
{
  mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void AbstractSprite::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, {*mBindGroup.descriptorSet}, {});
  commandBuffer.draw(6, 1, 0, 0);
}

Sprite::Sprite(const Device& device, Texture& texture)
    : AbstractSprite(device, SPIRV::TexturePosition_frag, texture)
{
}

}  // namespace Renderer
}  // namespace Vortex2D
