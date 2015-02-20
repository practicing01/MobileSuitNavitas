/*
 * SceneObjectRotateTo.cpp
 *
 *  Created on: Jan 3, 2015
 *      Author: practicing01, dahlia, hd_, others from the internet
 */

#include "math.h"
#include "ProcessUtils.h"
#include "Scene.h"
#include "SceneEvents.h"
#include "SceneObjectRotateTo.h"

#include "DebugNew.h"

SceneObjectRotateTo::SceneObjectRotateTo(Context* context) :
LogicComponent(context)
{
	isRotating_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void SceneObjectRotateTo::OnRotateToComplete()
{
	VariantMap vm;
	vm[SceneObjectRotateToComplete::P_NODE] = node_;
	SendEvent(E_SCENEOBJECTROTATETOCOMPLETE,vm);
}

void SceneObjectRotateTo::RotateTo(Quaternion dest, float speed, bool stopOnCompletion)
{
	rotateToDest_ = dest.Normalized();
	rotateToLoc_ = node_->GetRotation().Normalized();
	rotateToSpeed_ = speed * 360.0f;

	Quaternion vasDeferens = rotateToLoc_.Inverse() * rotateToDest_;
	float angel = 2.0f * Acos(vasDeferens.w_);
	rotateToTravelTime_ = angel / rotateToSpeed_;

	rotateToElapsedTime_ = 0.0f;
	rotateToStopOnTime_ = stopOnCompletion;
	isRotating_ = true;
}

void SceneObjectRotateTo::Update(float timeStep)
{
	if (isRotating_ == true)
	{
		inderp_ = 1.0f - (rotateToTravelTime_ - rotateToElapsedTime_);
		node_->SetRotation(rotateToLoc_.Slerp(rotateToDest_, inderp_));
		rotateToElapsedTime_ += timeStep;
		if (rotateToElapsedTime_ >= rotateToTravelTime_)
		{
			isRotating_ = false;
			if (rotateToStopOnTime_ == true)
			{
			}
			OnRotateToComplete();
		}
	}
}
