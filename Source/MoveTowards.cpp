/*
 * MoveTowards.cpp
 *
 *  Created on: Feb 4, 2015
 *      Author: practicing01
 */

#include "Node.h"
#include "MoveTowards.h"

MoveTowards::MoveTowards(Context* context) :
LogicComponent(context)
{
	isMoving_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void MoveTowards::MoveToward(Vector3 direction, float speed)
{
	isMoving_ = true;
	speed_ = speed;
	direction_ = direction.Normalized();
}

void MoveTowards::Stop()
{
	isMoving_ = false;
}

void MoveTowards::Update(float timeStep)
{
	if (isMoving_)
	{
		node_->SetPosition(node_->GetPosition() + (direction_ * (speed_ * timeStep) ) );
	}
}
