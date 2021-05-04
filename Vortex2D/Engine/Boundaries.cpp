//
//  Boundaries.cpp
//  Vortex2D
//

#include "Boundaries.h"

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
namespace
{
bool IsClockwise(const std::vector<glm::vec2>& points)
{
  float total = 0.0f;
  for (std::size_t i = points.size() - 1, j = 0; j < points.size(); i = j++)
  {
    total += (points[j].x - points[i].x) * (points[i].y + points[j].y);
  }

  return total > 0.0;
}

std::vector<glm::vec2> GetBoundingBox(const std::vector<glm::vec2>& points, float extent)
{
  glm::vec2 topLeft(std::numeric_limits<float>::max());
  glm::vec2 bottomRight(std::numeric_limits<float>::min());

  for (auto& point : points)
  {
    topLeft.x = glm::min(topLeft.x, point.x);
    topLeft.y = glm::min(topLeft.y, point.y);

    bottomRight.x = glm::max(bottomRight.x, point.x);
    bottomRight.y = glm::max(bottomRight.y, point.y);
  }

  topLeft -= glm::vec2(extent);
  bottomRight += glm::vec2(extent);

  return {{topLeft.x, topLeft.y},
          {bottomRight.x, topLeft.y},
          {topLeft.x, bottomRight.y},
          {
              bottomRight.x,
              topLeft.y,
          },
          {bottomRight.x, bottomRight.y},
          {topLeft.x, bottomRight.y}};
}

}  // namespace

Polygon::Polygon(const Renderer::Device& device,
                 std::vector<glm::vec2> points,
                 bool inverse,
                 float extent)
    : mDevice(const_cast<Renderer::Device&>(device))  // FIXME remove const_cast
    , mSize(static_cast<uint32_t>(points.size()))
    , mInv(inverse)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6ul)
    , mPolygonVertexBuffer(device, static_cast<unsigned>(points.size()))
{
  assert(!IsClockwise(points));
  if (inverse)
  {
    std::reverse(points.begin(), points.end());
  }

  Renderer::Buffer<glm::vec2> localPolygonVertexBuffer(
      device, points.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localPolygonVertexBuffer, points);

  auto boundingBox = GetBoundingBox(points, extent);
  Renderer::Buffer<glm::vec2> localVertexBuffer(
      device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localVertexBuffer, boundingBox);

  device.Execute([&](vk::CommandBuffer commandBuffer) {
    mPolygonVertexBuffer.CopyFrom(commandBuffer, localPolygonVertexBuffer);
    mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
  });

  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::PolygonDist_frag);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(
      bindGroupLayout, layout, {{mMVPBuffer}, {mMVBuffer}, {mPolygonVertexBuffer}});

  mPipeline =
      Renderer::GraphicsPipelineDescriptor()
          .Topology(vk::PrimitiveTopology::eTriangleList)
          .Shader(device.GetShaderModule(SPIRV::Position_vert), vk::ShaderStageFlagBits::eVertex)
          .Shader(device.GetShaderModule(SPIRV::PolygonDist_frag),
                  vk::ShaderStageFlagBits::eFragment)
          .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
          .VertexBinding(0, sizeof(glm::vec2))
          .Layout(mPipelineLayout);
}

Polygon::~Polygon() {}

void Polygon::Initialize(const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void Polygon::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Polygon::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  commandBuffer.pushConstants(mPipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
  commandBuffer.pushConstants(mPipelineLayout, vk::ShaderStageFlagBits::eFragment, 4, 4, &mInv);
  commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, {*mBindGroup.descriptorSet}, {});
  commandBuffer.draw(6, 1, 0, 0);
}

Rectangle::Rectangle(const Renderer::Device& device,
                     const glm::vec2& size,
                     bool inverse,
                     float extent)
    : Polygon(device,
              {{0.0f, 0.0f}, {size.x, 0.0f}, {size.x, size.y}, {0.0f, size.y}},
              inverse,
              extent)
{
}

Rectangle::~Rectangle() {}

void Rectangle::Initialize(const Renderer::RenderState& renderState)
{
  Polygon::Initialize(renderState);
}

void Rectangle::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Polygon::Update(projection, view);
}

void Rectangle::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  Polygon::Draw(commandBuffer, renderState);
}

Circle::Circle(const Renderer::Device& device, float radius, float extent)
    : mDevice(const_cast<Renderer::Device&>(device))  // FIXME remove const_cast
    , mSize(radius)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
{
  std::vector<glm::vec2> points = {
      {-radius, -radius}, {radius, -radius}, {radius, radius}, {-radius, radius}};

  auto boundingBox = GetBoundingBox(points, extent);
  Renderer::Buffer<glm::vec2> localVertexBuffer(
      device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localVertexBuffer, boundingBox);

  device.Execute([&](vk::CommandBuffer commandBuffer) {
    mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
  });

  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::CircleDist_frag);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(bindGroupLayout, layout, {{mMVPBuffer}, {mMVBuffer}});

  mPipeline =
      Renderer::GraphicsPipelineDescriptor()
          .Topology(vk::PrimitiveTopology::eTriangleList)
          .Shader(device.GetShaderModule(SPIRV::Position_vert), vk::ShaderStageFlagBits::eVertex)
          .Shader(device.GetShaderModule(SPIRV::CircleDist_frag),
                  vk::ShaderStageFlagBits::eFragment)
          .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
          .VertexBinding(0, sizeof(glm::vec2))
          .Layout(mPipelineLayout);
}

Circle::~Circle() {}

void Circle::Initialize(const Renderer::RenderState& renderState)
{
  mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void Circle::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Circle::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  commandBuffer.pushConstants(mPipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
  commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, {*mBindGroup.descriptorSet}, {});
  commandBuffer.draw(6, 1, 0, 0);
}

Renderer::ColorBlendState IntersectionBlend = [] {
  Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setColorBlendOp(vk::BlendOp::eMax)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eOne)
      .setColorWriteMask(vk::ColorComponentFlagBits::eR);

  return blendState;
}();

Renderer::ColorBlendState UnionBlend = [] {
  Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setColorBlendOp(vk::BlendOp::eMin)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eOne)
      .setColorWriteMask(vk::ColorComponentFlagBits::eR);

  return blendState;
}();

Vortex2D::Renderer::Clear BoundariesClear = Vortex2D::Renderer::Clear({10000.0f, 0.0f, 0.0f, 0.0f});

DistanceField::DistanceField(const Renderer::Device& device,
                             Renderer::RenderTexture& levelSet,
                             float scale)
    : Renderer::AbstractSprite(device, SPIRV::DistanceField_frag, levelSet), mScale(scale)
{
}

DistanceField::DistanceField(DistanceField&& other)
    : Renderer::AbstractSprite(std::move(other)), mScale(other.mScale)
{
}

DistanceField::~DistanceField() {}

void DistanceField::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  PushConstant(commandBuffer, 0, mScale);
  AbstractSprite::Draw(commandBuffer, renderState);
}

}  // namespace Fluid
}  // namespace Vortex2D
