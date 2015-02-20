/*
 * LifeTime.cpp
 *
 *  Created on: Feb 1, 2015
 *      Author: practicing01
 */

#include "Scene.h"
#include "SceneEvents.h"

#include "LifeTime.h"

LifeTime::LifeTime(Context* context) :
LogicComponent(context)
{
	delay_ = 0.0f;
	elapsedTime_ = 0.0f;
	living_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

LifeTime::LifeTime(Context* context, float delay) :
LogicComponent(context)
{
	delay_ = delay;
	elapsedTime_ = 0.0f;
	living_ = true;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void LifeTime::Update(float timeStep)
{
	if (living_)
	{
		elapsedTime_ += timeStep;
		if (elapsedTime_ >= delay_)
		{
			living_ = false;
			VariantMap vm;
			vm[LifeTimeComplete::P_NODE] = node_;
			SendEvent(E_LIFETIMECOMPLETE,vm);
		}
	}
}

void LifeTime::setLifeTime(float delay)
{
	elapsedTime_ = 0.0f;
	delay_ = delay;
	living_ = true;
}
