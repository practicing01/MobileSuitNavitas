/*
 * RotateTowards.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: practicing01
 */

#include "Node.h"
#include "RotateTowards.h"

RotateTowards::RotateTowards(Context* context) :
LogicComponent(context)
{
	isRotating_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void RotateTowards::RotateToward(Vector3 direction, float speed)
{
	isRotating_ = true;
	speed_ = speed;
	direction_ = direction.Normalized();
}

void RotateTowards::Stop()
{
	isRotating_ = false;
}

void RotateTowards::Update(float timeStep)
{
	if (isRotating_)
	{
		/*node_->Roll(direction_.z_ * (speed_ * timeStep));
		node_->Yaw(direction_.y_ * (speed_ * timeStep));
		return;*/
		quaterPounder_ = node_->GetRotation();
		quaterOnion_ = quaterPounder_ * Quaternion(direction_.y_ * (speed_ * timeStep), Vector3::UP);
		quaterPounder_ = quaterOnion_ * Quaternion(direction_.z_ * (speed_ * timeStep), Vector3::FORWARD);
		node_->SetRotation(quaterPounder_);
	}
}
