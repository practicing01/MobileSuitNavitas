/*
 * RotateTowards.h
 *
 *  Created on: Feb 6, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "LogicComponent.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class RotateTowards: public LogicComponent
{
	OBJECT(RotateTowards);
public:
	RotateTowards(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	void RotateToward(Vector3 direction, float speed);
	void Stop();

	bool isRotating_;
	float speed_;
	Vector3 direction_;
	Quaternion quaterPounder_;
	Quaternion quaterOnion_;
};
