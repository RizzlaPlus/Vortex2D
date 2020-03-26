//
//  Common.h
//  Vortex
//

#ifndef Vortex2D_Common_h
#define Vortex2D_Common_h

#include <Vortex2D/Renderer/Gpu.h>

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#ifdef _WIN32
#ifdef VORTEX2D_API_EXPORTS
#define VORTEX2D_API __declspec(dllexport)
#else
#define VORTEX2D_API __declspec(dllimport)
#endif
#else
#define VORTEX2D_API
#endif

#endif
