//
//  Texture.h
//  Vortex2D
//

#ifndef Vortex2d_Texture_h
#define Vortex2d_Texture_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D
{
namespace Renderer
{
class Device;
class CommandEncoder;

/**
 * @brief Gets the number of bytes per pixel given the format
 * @param format of texture
 * @return bytes per pixel
 */
VORTEX2D_API std::uint64_t GetBytesPerPixel(Format format);

class Sampler
{
public:
  enum class AddressMode
  {
    ClampToEdge,
    Repeat,
  };

  enum class Filter
  {
    Linear,
    Nearest,
  };

  Sampler(Device& device,
          AddressMode adressMode = AddressMode::Repeat,
          Filter filter = Filter::Nearest);

  Sampler(Sampler&& other);

  VORTEX2D_API ~Sampler();

  Handle::Sampler Handle();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

/**
 * @brief A texture, or in vulkan terms, an image.
 */
class Texture
{
public:
  VORTEX2D_API Texture(Device& device,
                       uint32_t width,
                       uint32_t height,
                       Format format,
                       MemoryUsage memoryUsage = MemoryUsage::Gpu);
  VORTEX2D_API Texture(Texture&& other);

  VORTEX2D_API virtual ~Texture();

  template <typename T>
  void CopyFrom(const std::vector<T>& data)
  {
    if (data.size() != GetWidth() * GetHeight())
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(GetFormat()))
      throw std::runtime_error("Invalid input data format");
    CopyFrom(data.data());
  }

  template <typename T>
  void CopyTo(std::vector<T>& data)
  {
    if (data.size() != GetWidth() * GetHeight())
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(GetFormat()))
      throw std::runtime_error("Invalid input data format");
    CopyTo(data.data());
  }

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data source data
   */
  VORTEX2D_API void CopyFrom(const void* data);

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data destination data
   */
  VORTEX2D_API void CopyTo(void* data);

  /**
   * @brief Copies source texture in this texture
   * @param commandBuffer vulkan command buffer
   * @param srcImage source image
   */
  VORTEX2D_API void CopyFrom(CommandEncoder& command, Texture& srcImage);

  /**
   * @brief Inserts a barrier for the given texture, command buffer and access.
   * @param image the vulkan image handle
   * @param commandBuffer the vulkan command buffer
   * @param oldLayout old layout
   * @param srcMask old access
   * @param newLayout new layout
   * @param dstMask new access
   */
  VORTEX2D_API void Barrier(CommandEncoder& command,
                            ImageLayout oldLayout,
                            Access oldAccess,
                            ImageLayout newLayout,
                            Access newAccess);

  VORTEX2D_API Handle::ImageView GetView() const;
  VORTEX2D_API uint32_t GetWidth() const;
  VORTEX2D_API uint32_t GetHeight() const;
  VORTEX2D_API Format GetFormat() const;

  VORTEX2D_API void Clear(CommandEncoder& command, const std::array<int, 4>& colour);
  VORTEX2D_API void Clear(CommandEncoder& command, const std::array<float, 4>& colour);

  VORTEX2D_API Handle::Image Handle() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
