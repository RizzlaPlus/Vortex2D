#ifdef VORTEX2D_BACKEND_VULKAN
#include <Vortex2D/Renderer/Vulkan/Device.h>
#include <Vortex2D/Renderer/Vulkan/Instance.h>
#else
#include <Vortex2D/Renderer/WebGPU/Device.h>
#include <Vortex2D/Renderer/WebGPU/Instance.h>
#endif
#include <gtest/gtest.h>

Vortex2D::Renderer::Device* device;

int main(int argc, char** argv)
{
#ifdef NDEBUG
  bool debug = false;
#else
  bool debug = true;
#endif

#ifdef VORTEX2D_BACKEND_VULKAN
  Vortex2D::Renderer::Instance instance("Tests", {}, debug);
  Vortex2D::Renderer::VulkanDevice device_(instance);
#else
  Vortex2D::Renderer::Instance instance;
  Vortex2D::Renderer::WebGPUDevice device_(instance);
#endif

  device = &device_;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
