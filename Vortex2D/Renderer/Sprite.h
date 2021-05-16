//
//  Sprite.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Transformable.h>

namespace Vortex
{
namespace Renderer
{
struct RenderTarget;

/**
 * @brief a Sprite, i.e. a drawable that can render a texture. The fragment
 * shader can be specified for customisation.
 */
class AbstractSprite : public Drawable, public Transformable
{
public:
  VORTEX_API AbstractSprite(const Device& device,
                            const SpirvBinary& fragShaderName,
                            Texture& texture);
  VORTEX_API AbstractSprite(AbstractSprite&& other);
  VORTEX_API virtual ~AbstractSprite() override;

  VORTEX_API void Initialize(const RenderState& renderState) override;
  VORTEX_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
  VORTEX_API void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

  template <typename T>
  void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
  {
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout,
                                vk::ShaderStageFlagBits::eFragment,
                                offset,
                                sizeof(T),
                                &data);
  }

  glm::vec4 Colour = {1.0f, 1.0f, 1.0f, 1.0f};

protected:
  struct Vertex
  {
    glm::vec2 uv;
    glm::vec2 pos;
  };

  const Device& mDevice;
  UniformBuffer<glm::mat4> mMVPBuffer;
  VertexBuffer<Vertex> mVertexBuffer;
  Renderer::UniformBuffer<glm::vec4> mColourBuffer;
  vk::UniqueSampler mSampler;
  DescriptorSet mDescriptorSet;
  GraphicsPipeline mPipeline;
};

/**
 * @brief A sprite that renders a texture with a simple pass-through fragment
 * shader.
 */
class Sprite : public AbstractSprite
{
public:
  VORTEX_API Sprite(const Device& device, Texture& texture);
};

}  // namespace Renderer
}  // namespace Vortex
