/*
 * LifeTime.h
 *
 *  Created on: Feb 1, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "LogicComponent.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

EVENT(E_LIFETIMECOMPLETE, LifeTimeComplete)
{
   PARAM(P_NODE, Node);  //node
}

class LifeTime: public LogicComponent
{
	OBJECT(LifeTime);
public:
	LifeTime(Context* context);
	LifeTime(Context* context, float delay);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	void setLifeTime(float delay);
	float delay_;
	float elapsedTime_;
	bool living_;
};
