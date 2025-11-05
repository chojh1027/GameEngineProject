/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Erin Catto makes no representations about the suitability 
* of this software for any purpose.  
* It is provided "as is" without express or implied warranty.
*/

#include "box2d-lite/Body.h"

Body::Body()
{
        position.Set(0.0f, 0.0f);
        rotation = 0.0f;
        velocity.Set(0.0f, 0.0f);
        angularVelocity = 0.0f;
        force.Set(0.0f, 0.0f);
        torque = 0.0f;
        friction = 0.2f;

        width.Set(1.0f, 1.0f);
        radius = 0.0f;
        shape = ShapeType::Box;
        mass = FLT_MAX;
        invMass = 0.0f;
        I = FLT_MAX;
        invI = 0.0f;
}

void Body::Set(const Vec2& w, float m, ShapeType shapeType)
{
        position.Set(0.0f, 0.0f);
        rotation = 0.0f;
        velocity.Set(0.0f, 0.0f);
        angularVelocity = 0.0f;
        force.Set(0.0f, 0.0f);
        torque = 0.0f;
        friction = 0.2f;

        width = w;
        shape = shapeType;
        radius = shape == ShapeType::Circle ? 0.5f * w.x : 0.0f;
        mass = m;

        if (mass < FLT_MAX)
        {
                invMass = 1.0f / mass;
                if (shape == ShapeType::Circle)
                {
                        I = mass * radius * radius * 0.5f;
                }
                else
                {
                        I = mass * (width.x * width.x + width.y * width.y) / 12.0f;
                }
                invI = I > 0.0f ? 1.0f / I : 0.0f;
        }
        else
        {
                invMass = 0.0f;
                I = FLT_MAX;
		invI = 0.0f;
	}
}
