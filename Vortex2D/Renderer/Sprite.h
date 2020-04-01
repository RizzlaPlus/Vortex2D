//
//  Sprite.h
//  Vortex2D
//

#ifndef Sprite_h
#define Sprite_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Transformable.h>

namespace Vortex2D
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
  VORTEX2D_API AbstractSprite(Device& device, const SpirvBinary& fragShaderName, Texture& texture);
  VORTEX2D_API AbstractSprite(AbstractSprite&& other);
  VORTEX2D_API virtual ~AbstractSprite() override;

  VORTEX2D_API void Initialize(const RenderState& renderState) override;
  VORTEX2D_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
  VORTEX2D_API void Draw(CommandEncoder& command, const RenderState& renderState) override;

  template <typename T>
  void PushConstant(Renderer::CommandEncoder& command, uint32_t offset, const T& data)
  {
    command.PushConstants(
        mPipelineLayout, Renderer::ShaderStage::Fragment, offset, sizeof(T), &data);
  }

  glm::vec4 Colour = {1.0f, 1.0f, 1.0f, 1.0f};

protected:
  struct Vertex
  {
    glm::vec2 uv;
    glm::vec2 pos;
  };

  Device& mDevice;
  UniformBuffer<glm::mat4> mMVPBuffer;
  VertexBuffer<Vertex> mVertexBuffer;
  Renderer::UniformBuffer<glm::vec4> mColourBuffer;
  Renderer::Sampler mSampler;
  vk::PipelineLayout mPipelineLayout;
  BindGroup mBindGroup;
  GraphicsPipelineDescriptor mPipeline;
};

/**
 * @brief A sprite that renders a texture with a simple pass-through fragment
 * shader.
 */
class Sprite : public AbstractSprite
{
public:
  VORTEX2D_API Sprite(Device& device, Texture& texture);
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
