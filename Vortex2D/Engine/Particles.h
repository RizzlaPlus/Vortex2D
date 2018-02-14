//
//  Particles.h
//  Vortex
//

#ifndef Vortex2d_Particles_h
#define Vortex2d_Particles_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Engine/PrefixScan.h>

namespace Vortex2D { namespace Fluid {

class LevelSet;

struct Particle
{
  alignas(8) glm::vec2 Position;
  alignas(8) glm::vec2 Velocity;
};

class ParticleCount : public Renderer::RenderTexture
{
public:
    ParticleCount(const Renderer::Device& device,
                  const glm::ivec2& size,
                  Renderer::GenericBuffer& particles,
                  const Renderer::DispatchParams& params = {0});

    void Scan();

    int GetTotalCount();
    Renderer::GenericBuffer& GetDispatchParams();

    void InitLevelSet(LevelSet& levelSet);
    void Phi();

    void InitVelocities(Renderer::Texture& velocity, Renderer::GenericBuffer& valid);
    void TransferToGrid();
    void TransferFromGrid();

private:
    const Renderer::Device& mDevice;
    Renderer::GenericBuffer& mParticles;
    Renderer::Buffer<Particle> mNewParticles;
    Renderer::Buffer<int> mDelta, mCount;
    Renderer::Buffer<int> mIndex;
    Renderer::Buffer<glm::ivec2> mSeeds;

    Renderer::IndirectBuffer<Renderer::DispatchParams> mDispatchParams;
    Renderer::Buffer<Renderer::DispatchParams> mLocalDispatchParams, mNewDispatchParams;

    Renderer::Work mParticleCountWork;
    Renderer::Work::Bound mParticleCountBound;
	Renderer::Work mParticleClampWork;
	Renderer::Work::Bound mParticleClampBound;
    PrefixScan mPrefixScan;
    PrefixScan::Bound mPrefixScanBound;
    Renderer::Work mParticleBucketWork;
    Renderer::Work::Bound mParticleBucketBound;
    Renderer::Work mParticleSpawnWork;
    Renderer::Work::Bound mParticleSpawnBound;
    Renderer::Work mParticlePhiWork;
    Renderer::Work::Bound mParticlePhiBound;
    Renderer::Work mParticleToGridWork;
    Renderer::Work::Bound mParticleToGridBound;
    Renderer::Work mParticleFromGridWork;
    Renderer::Work::Bound mParticleFromGridBound;

    Renderer::CommandBuffer mScanWork;
    Renderer::CommandBuffer mDispatchCountWork;
    Renderer::CommandBuffer mParticlePhi;
    Renderer::CommandBuffer mParticleToGrid;
    Renderer::CommandBuffer mParticleFromGrid;
};

}}

#endif
