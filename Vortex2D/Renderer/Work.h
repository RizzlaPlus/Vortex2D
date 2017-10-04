//
//  Work.h
//  Vortex2D
//

#ifndef Vortex2D_Work_h
#define Vortex2D_Work_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include <Vortex2D/Utils/variant.hpp>

namespace Vortex2D { namespace Renderer {

struct ComputeSize
{
    static glm::ivec2 GetLocalSize2D();
    static int GetLocalSize1D();

    static glm::ivec2 GetWorkSize(const glm::ivec2& size);
    static glm::ivec2 GetWorkSize(int size);

    static ComputeSize Default2D();
    static ComputeSize Default1D();

    ComputeSize(const glm::ivec2& size);
    ComputeSize(int size);

    glm::ivec2 DomainSize;
    glm::ivec2 WorkSize;
    glm::ivec2 LocalSize;
};

ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius);
ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size);

struct DispatchParams
{
    DispatchParams(int count);
    alignas(16) vk::DispatchIndirectCommand workSize;
    alignas(4) uint32_t count;
};

class Work
{
public:
    struct Input
    {
        Input(Renderer::Buffer& buffer);
        Input(Renderer::Texture& texture);
        Input(vk::Sampler sampler, Renderer::Texture& texture);

        struct DescriptorImage
        {
            DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture);
            DescriptorImage(Renderer::Texture& texture);

            vk::Sampler Sampler;
            Renderer::Texture* Texture;
        };

        mpark::variant<Renderer::Buffer*, DescriptorImage> Bind;
    };

    Work(const Device& device,
         const ComputeSize& computeSize,
         const std::string& shader);

    class Bound
    {
    public:
        Bound();

        template<typename T>
        void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
        {
            if (offset + sizeof(T) <= mPushConstantSize)
            {
                commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(T), &data);
            }
        }

        void Record(vk::CommandBuffer commandBuffer);
        void RecordIndirect(vk::CommandBuffer commandBuffer, Buffer& dispatchParams);

        friend class Work;

    private:
        Bound(const ComputeSize& computeSize,
              uint32_t pushConstantSize,
              vk::PipelineLayout layout,
              vk::Pipeline pipeline,
              vk::UniqueDescriptorSet descriptor);

        ComputeSize mComputeSize;
        uint32_t mPushConstantSize;
        vk::PipelineLayout mLayout;
        vk::Pipeline mPipeline;
        vk::UniqueDescriptorSet mDescriptor;
    };

    // TODO save the bound inside the Work class and access with other method
    Bound Bind(const std::vector<Input>& inputs);
    Bound Bind(ComputeSize computeSize, const std::vector<Input>& inputs);

private:
    ComputeSize mComputeSize;
    uint32_t mPushConstantSize;
    const Device& mDevice;
    vk::DescriptorSetLayout mDescriptorLayout;
    vk::UniquePipelineLayout mLayout;
    vk::UniquePipeline mPipeline;
    std::vector<vk::DescriptorType> mBindings;
};

}}

#endif
