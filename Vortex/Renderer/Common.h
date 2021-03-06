//
//  Common.h
//  Vortex
//

#pragma once

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

#define VMA_RECORDING_ENABLED 0

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#ifdef _WIN32
#ifdef VORTEX2D_API_EXPORTS
#define VORTEX_API __declspec(dllexport)
#else
#define VORTEX_API __declspec(dllimport)
#endif
#else
#define VORTEX_API
#endif
