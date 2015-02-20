/*
 * SimulationMenu.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: practicing01
 */

#include "CoreEvents.h"
#include "Camera.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Graphics.h"
#include "InputEvents.h"
#include "Node.h"
#include "Octree.h"
#include "OctreeQuery.h"
#include "Renderer.h"
#include "RenderPath.h"
#include "ResourceCache.h"
#include "Scene.h"
#include "UI.h"
#include "Viewport.h"

//#include "Log.h"
//#include "DebugNew.h"

#include "SpaceSimulationMenuSelect.h"
#include "SimulationMenu.h"

SimulationMenu::SimulationMenu(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;
	simulationIndex_ = 0;

	main_->cameraNode_->RemoveAllChildren();
	main_->cameraNode_->RemoveAllComponents();
	main_->cameraNode_->Remove();

	main_->renderer_->SetNumViewports(2);

	File loadFile(context_,
			main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/MSNMenu.xml", FILE_READ);
	main_->scene_->LoadXML(loadFile);

	main_->cameraNode_ = main_->scene_->GetChild("camera");
	main_->viewport_->SetCamera(main_->cameraNode_->GetComponent<Camera>());

	main_->renderer_->SetViewport(0, main_->viewport_);

	logicStates_.Push(new SpaceSimulationMenuSelect(context_, main_));

	logicStates_[0]->SendEvent(E_LOADMENUSCENE);

	SubscribeToEvent(E_TOUCHBEGIN, HANDLER(SimulationMenu, TouchDown));
	SubscribeToEvent(E_UPDATE, HANDLER(SimulationMenu, HandleUpdate));
}

SimulationMenu::~SimulationMenu()
{
}

void SimulationMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("loop");
	elapsedTime_ += timeStep;

}

void SimulationMenu::TouchDown(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchBegin;

	Ray cameraRay = main_->cameraNode_->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	PODVector<RayQueryResult> results;

	RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f,
			DRAWABLE_GEOMETRY);

	main_->scene_->GetComponent<Octree>()->Raycast(query);

	if (results.Size())
	{
		for (int x = 0; x < results.Size(); x++)
		{
			//LOGERRORF(results[x].node_->GetName().CString());
			if (results[x].node_->GetName() == "loadButt")
			{
				//do stuffs with butt
				main_->scene_->RemoveAllChildren();
				main_->scene_->RemoveAllComponents();
				main_->logicStates_.Remove(this);

				logicStates_[simulationIndex_]->SendEvent(E_UNLOADMENUSCENE);
				logicStates_[simulationIndex_]->SendEvent(E_LOADSIMULATION);

				delete this;
			}
			else if (results[x].node_->GetName() == "upButt")
			{
				//decrease index, clear secondary scene, call secondary scene load
			}
			else if (results[x].node_->GetName() == "downButt")
			{
				//increase index, clear secondary scene, call secondary scene load
			}
		}
	}
	else
	{
		return;
	}

}
