//
//  Boundaries.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>
#include <Vortex/Renderer/RenderTexture.h>
#include <Vortex/Renderer/Shapes.h>
#include <Vortex/Renderer/Sprite.h>
#include <Vortex/Renderer/Transformable.h>
#include <Vortex/Renderer/Work.h>

#include <Vortex/Engine/Particles.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Signed distance field of a poylgon.
 */
class Polygon : public Renderer::Transformable, public Renderer::Drawable
{
public:
  /**
   * @brief Initialize polygon with set of points and extent of signed distance
   * @param device vulkan device
   * @param points clockwise oriented set of points (mininum 3).
   * @param inverse flag if the distance field should be inversed.
   * @param extent extend how far from the poylon the signed distance field is
   * calculated.
   */
  VORTEX_API Polygon(const Renderer::Device& device,
                     std::vector<glm::vec2> points,
                     bool inverse = false,
                     float extent = 10.0f);

  VORTEX_API ~Polygon() override;

  VORTEX_API void Initialize(const Renderer::RenderState& renderState) override;
  VORTEX_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
  VORTEX_API void Draw(vk::CommandBuffer commandBuffer,
                       const Renderer::RenderState& renderState) override;

private:
  const Renderer::Device& mDevice;
  uint32_t mSize;
  uint32_t mInv;
  Renderer::UniformBuffer<glm::mat4> mMVPBuffer;
  Renderer::UniformBuffer<glm::mat4> mMVBuffer;
  Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
  Renderer::DescriptorSet mDescriptorSet;
  Renderer::GraphicsPipeline mPipeline;
  Renderer::Buffer<glm::vec2> mPolygonVertexBuffer;
};

/**
 * @brief Signed distance field of a rectangle.
 */
class Rectangle : public Polygon
{
public:
  /**
   * @brief Initialize rectangle with size and extend of signed distance.
   * @param device vulkan device.
   * @param size rectangle size
   * @param inverse flag if the distance field should be inverted.
   * @param extent extent how far from the rectangle the signed distance field
   * is calculated.
   */
  VORTEX_API Rectangle(const Renderer::Device& device,
                       const glm::vec2& size,
                       bool inverse = false,
                       float extent = 10.0f);

  VORTEX_API ~Rectangle();

  VORTEX_API void Initialize(const Renderer::RenderState& renderState) override;
  VORTEX_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
  VORTEX_API void Draw(vk::CommandBuffer commandBuffer,
                       const Renderer::RenderState& renderState) override;
};

/**
 * @brief Signed distance field of circle.
 */
class Circle : public Renderer::Transformable, public Renderer::Drawable
{
public:
  /**
   * @brief Initialize the circle with radius and extend of signed distance.
   * @param device vulkan device.
   * @param radius radius of circle.
   * @param extent extend how far from the circle the signed distance field is
   * calculated.
   */
  VORTEX_API Circle(const Renderer::Device& device, float radius, float extent = 10.0f);

  VORTEX_API ~Circle() override;

  VORTEX_API void Initialize(const Renderer::RenderState& renderState) override;
  VORTEX_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
  VORTEX_API void Draw(vk::CommandBuffer commandBuffer,
                       const Renderer::RenderState& renderState) override;

private:
  const Renderer::Device& mDevice;
  float mSize;
  Renderer::UniformBuffer<glm::mat4> mMVPBuffer;
  Renderer::UniformBuffer<glm::mat4> mMVBuffer;
  Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
  Renderer::DescriptorSet mDescriptorSet;
  Renderer::GraphicsPipeline mPipeline;
};

extern VORTEX_API Renderer::ColorBlendState IntersectionBlend;
extern VORTEX_API Renderer::ColorBlendState UnionBlend;
extern VORTEX_API Renderer::Clear BoundariesClear;

/**
 * @brief Sprite of a distance field
 */
class DistanceField : public Renderer::AbstractSprite
{
public:
  /**
   * @brief Initialize the price with the level set and scale
   * @param device vulkan device
   * @param levelSet level set to use as sprite
   * @param scale scale of the level set
   */
  VORTEX_API DistanceField(const Renderer::Device& device,
                           Renderer::RenderTexture& levelSet,
                           float scale = 1.0f);

  VORTEX_API DistanceField(DistanceField&& other);

  VORTEX_API ~DistanceField() override;

  VORTEX_API void Draw(vk::CommandBuffer commandBuffer,
                       const Renderer::RenderState& renderState) override;

private:
  float mScale;
};

}  // namespace Fluid
}  // namespace Vortex
