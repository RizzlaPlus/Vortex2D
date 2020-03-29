//
//  Timer.cpp
//  Vortex2D
//

#include "Timer.h"

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Device.h>

namespace Vortex2D
{
namespace Renderer
{
namespace
{
uint64_t GetMask(uint32_t validBits)
{
  if (validBits == 64)
  {
    return static_cast<uint64_t>(-1);
  }
  else
  {
    return (1 << validBits) - 1;
  }
}
}  // namespace

Timer::Timer(Device& device) : mDevice(device), mStart(device), mStop(device)
{
  auto queryPoolInfo =
      vk::QueryPoolCreateInfo().setQueryType(vk::QueryType::eTimestamp).setQueryCount(2);

  mPool = device.Handle().createQueryPoolUnique(queryPoolInfo);

  mStart.Record([&](CommandEncoder& command) {
    command.Handle().resetQueryPool(*mPool, 0, 2);
    command.Handle().writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
  });

  mStop.Record([&](CommandEncoder& command) {
    command.Handle().writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 1);
  });
}

void Timer::Start(CommandEncoder& command)
{
  command.Handle().resetQueryPool(*mPool, 0, 2);
  command.Handle().writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
}

void Timer::Stop(CommandEncoder& command)
{
  command.Handle().writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 1);
}

void Timer::Start()
{
  mStart.Submit();
}

void Timer::Stop()
{
  mStop.Submit();
}

void Timer::Wait()
{
  mStart.Wait();
  mStop.Wait();
}

uint64_t Timer::GetElapsedNs()
{
  uint64_t timestamps[2] = {0};
  auto result = mDevice.Handle().getQueryPoolResults(
      *mPool,
      0,
      2,
      sizeof(timestamps),
      timestamps,
      sizeof(uint64_t),
      vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64);

  if (result == vk::Result::eSuccess)
  {
    int familyIndex = mDevice.GetFamilyIndex();
    auto properties = mDevice.GetPhysicalDevice().getProperties();
    assert(properties.limits.timestampComputeAndGraphics);

    uint64_t period = static_cast<uint64_t>(properties.limits.timestampPeriod);

    auto queueProperties = mDevice.GetPhysicalDevice().getQueueFamilyProperties();
    auto validBits = queueProperties[familyIndex].timestampValidBits;

    return ((timestamps[1] & GetMask(validBits)) - (timestamps[0] & GetMask(validBits))) * period;
  }
  else
  {
    return static_cast<uint64_t>(-1);
  }
}

}  // namespace Renderer
}  // namespace Vortex2D
