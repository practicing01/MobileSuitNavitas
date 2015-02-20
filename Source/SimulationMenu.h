/*
 * SimulationMenu.h
 *
 *  Created on: Jan 11, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "Urho3DPlayer.h"

using namespace Urho3D;

static const StringHash E_LOADMENUSCENE("LoadMenuScene");

static const StringHash E_UNLOADMENUSCENE("UnloadMenuScene");

static const StringHash E_LOADSIMULATION("LoadSimulation");

class SimulationMenu : public Object
{
	OBJECT(SimulationMenu);
public:
	SimulationMenu(Context* context, Urho3DPlayer* main);
	~SimulationMenu();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void TouchDown(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	float elapsedTime_;
	int simulationIndex_;
	Vector<Object*> logicStates_;

};
