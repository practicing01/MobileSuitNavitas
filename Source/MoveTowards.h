/*
 * MoveTowards.h
 *
 *  Created on: Feb 4, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "LogicComponent.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class MoveTowards: public LogicComponent
{
	OBJECT(MoveTowards);
public:
	MoveTowards(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	void MoveToward(Vector3 direction, float speed);
	void Stop();

	bool isMoving_;
	float speed_;
	Vector3 direction_;
};
