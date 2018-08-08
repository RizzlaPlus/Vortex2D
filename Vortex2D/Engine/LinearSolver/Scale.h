//
//  Scale.h
//  Vortex2D
//

#ifndef Vortex2d_Scale_h
#define Vortex2d_Scale_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Scale a level set down to a coarser level set.
 */
class Scale
{
public:
    /**
     * @brief Initialize prolongate and restrict compute pipelines
     * @param device
     * @param numFineGrid
     */
    VORTEX2D_API Scale(const Renderer::Device& device, int numFineGrid);

    /**
     * @brief down sample a level set on a finer level set.
     * Multiple level sets can be bound and indexed.
     * @param level the index of the bound level set to prolongate
     * @param fineSize size of the finer level set
     * @param fine the finer level set
     * @param coarse the coarse level set
     */
    VORTEX2D_API void DownSampleBind(std::size_t level,
                                     const glm::ivec2& fineSize,
                                     Renderer::Texture& fine,
                                     Renderer::Texture& coarse);
    /**
     * @brief Down sample the level set, using the bound level sets at the specified index.
     * @param commandBuffer command buffer to record into.
     * @param level index of bound level sets.
     */
    VORTEX2D_API void DownSample(vk::CommandBuffer commandBuffer, std::size_t level);

private:
    const Renderer::Device& mDevice;
    int mNumFineGrid;
    Renderer::Work mDownSampleWork;
    Renderer::Work mDownSamplePositiveWork;
    std::vector<Renderer::Work::Bound> mDownSampleBound;
    std::vector<Renderer::Texture*> mDownSampleBuffer;
};

}}

#endif
