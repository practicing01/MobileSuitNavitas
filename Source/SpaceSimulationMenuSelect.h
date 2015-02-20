/*
 * SpaceSimulationMenuSelect.h
 *
 *  Created on: Jan 11, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "Urho3DPlayer.h"

using namespace Urho3D;

class SpaceSimulationMenuSelect : public Object
{
	OBJECT(SpaceSimulationMenuSelect);
public:
	SpaceSimulationMenuSelect(Context* context, Urho3DPlayer* main);
	~SpaceSimulationMenuSelect();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void LoadMenuScene(StringHash eventType, VariantMap& eventData);
	void UnloadMenuScene(StringHash eventType, VariantMap& eventData);
	void LoadSimulation(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	float elapsedTime_;

	SharedPtr<Viewport> viewport_;
	SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
};
