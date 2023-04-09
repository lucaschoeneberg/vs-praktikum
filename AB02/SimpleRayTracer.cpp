//
//  SimpleRayTracer.cpp
//  SimpleRayTracer
//
//  Created by Philipp Lensing on 03.09.14.
//  Copyright (c) 2014 Philipp Lensing. All rights reserved.
//

#include "SimpleRayTracer.h"
#include "rgbimage.h"
#include <stdio.h>
#include <math.h>


Camera::Camera( float zvalue, float planedist, float width, float height, unsigned int widthInPixel, unsigned int heightInPixel )
{
	// TODO: Add your code
}

Vector Camera::generateRay( unsigned int x, unsigned int y) const
{
	// TODO: Add your code
    return Vector(); // dummy (remove)
}

Vector Camera::Position() const
{
	// TODO: Add your code
	return Vector(); // dummy (remove)
}

SimpleRayTracer::SimpleRayTracer(unsigned int MaxDepth)
{
	// TODO: Add your code
}


void SimpleRayTracer::traceScene( const Scene& SceneModel, RGBImage& Image)
{
	// TODO: Add your code
}

Color SimpleRayTracer::localIllumination( const Vector& Surface, const Vector& Eye, const Vector& N, const PointLight& Light, const Material& Mtrl )
{
	// TODO: Add your code
	return Color(); // dummy (remove)
}

Color SimpleRayTracer::trace( const Scene& SceneModel, const Vector& o, const Vector& d, int depth)
{
	// TODO: Add your code
	return Color(); // dummy (remove)
}

