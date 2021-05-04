//
//  Timer.h
//  Vortex2D
//

#ifndef Vortex2D_Timer_h
#define Vortex2D_Timer_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Calculates the ellapsed time on the GPU.
 */
class Timer
{
public:
  VORTEX2D_API Timer(Device& device);
  VORTEX2D_API ~Timer();

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX2D_API void Start(CommandEncoder& command);

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX2D_API void Stop(CommandEncoder& command);

  /**
   * @brief Start the timer after the current last command buffer
   */
  VORTEX2D_API void Start();

  /**
   * @brief Stop the timer after the current last command buffer
   */
  VORTEX2D_API void Stop();

  /**
   * @brief Wait for @ref Start and @ref Stop to finish before retrieving the
   * results
   */
  VORTEX2D_API void Wait();

  /**
   * @brief Get the elapsed time between the Start and Stop calls.
   * Blocking function which will download the timestamps from the GPU.
   * @return timestamp in nanoseconds.
   */
  VORTEX2D_API uint64_t GetElapsedNs();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
