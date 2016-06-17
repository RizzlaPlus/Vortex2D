//
//  Water.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Water__
#define __Vertex2D__Water__

#include "Drawable.h"
#include "Transformable.h"
#include "Size.h"
#include "Operator.h"
#include "Buffer.h"


namespace Fluid
{

class Engine;

/**
 * @brief Class to simulate water using the LevelSet method. This contains a full size grid
 * with sections marked as water and others marked as air.
 */
class Water : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Water(Dimensions dimensions);

    /**
     * @brief Renders water section.
     * @param object needs to draw with colour (1, 0, 0, 0)
     */
    void Render(Renderer::Drawable & object);

    /**
     * @brief Renders the water on a RenderTarget
     * @param target
     * @param transform
     */
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;

    /**
     * @brief Clears the LevelSet
     */
    void Clear();

    /**
     * @brief Gets the air section of the water. Has to be used to mark dirichlet boundaries in the engine.
     * @return returns a Sprite used to draw the dirichlet boundaries.
     */
    Renderer::Sprite & GetBoundaries();

    /**
     * @brief Advects the LevelSet
     * @param engine
     */
    void Advect(Engine & engine);

    /**
     * @brief Colour of the water. The air is transparent.
     */
    glm::vec4 Colour;

private:
    void Redistance(bool reinitialize = false);

    Dimensions mDimensions;
    Buffer mLevelSet;
    Buffer mSignLevelSet;
    Operator mRedistance;
    Operator mSign;
    
    Renderer::Program mLevelSetMask;
    Renderer::Program mRenderProgram;

    Renderer::Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vertex2D__Water__) */