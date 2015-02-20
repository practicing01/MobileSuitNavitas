/*
 * SpaceSimulationMenuSelect.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: practicing01
 */

#include "CoreEvents.h"
#include "Camera.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Graphics.h"
#include "Node.h"
#include "Octree.h"
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
#include "SpaceSimulation.h"

SpaceSimulationMenuSelect::SpaceSimulationMenuSelect(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

	SubscribeToEvent(E_LOADMENUSCENE,
			HANDLER(SpaceSimulationMenuSelect, LoadMenuScene));

	SubscribeToEvent(E_UNLOADMENUSCENE,
				HANDLER(SpaceSimulationMenuSelect, UnloadMenuScene));

	SubscribeToEvent(E_LOADSIMULATION,
				HANDLER(SpaceSimulationMenuSelect, LoadSimulation));
}

SpaceSimulationMenuSelect::~SpaceSimulationMenuSelect()
{
}

void SpaceSimulationMenuSelect::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("loop");
	elapsedTime_ += timeStep;

}

void SpaceSimulationMenuSelect::LoadMenuScene(StringHash eventType, VariantMap& eventData)
{

	scene_ = new Scene(main_->GetContext());
	cameraNode_ = new Node(main_->GetContext());

	File loadFile2(context_,
			main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/spaceSimulationMenu.xml", FILE_READ);
	scene_->LoadXML(loadFile2);

	cameraNode_ = scene_->GetChild("camera");

	SharedPtr<RenderPath> colorUnclearRenderPath = SharedPtr<RenderPath> (new RenderPath());
	colorUnclearRenderPath->Load(main_->cache_->GetResource<XMLFile>("RenderPaths/BackgroundLayer.xml"));
	viewport_ = new Viewport(main_->GetContext(), scene_, cameraNode_->GetComponent<Camera>());
	viewport_->SetCamera(cameraNode_->GetComponent<Camera>());
	viewport_->SetRenderPath(colorUnclearRenderPath);
	main_->renderer_->SetViewport(1, viewport_);

	SubscribeToEvent(E_UPDATE, HANDLER(SpaceSimulationMenuSelect, HandleUpdate));
}

void SpaceSimulationMenuSelect::UnloadMenuScene(StringHash eventType, VariantMap& eventData)
{
	if (scene_)
	{
		scene_->RemoveAllChildren();
		scene_->RemoveAllComponents();
		scene_->Remove();
	}
}

void SpaceSimulationMenuSelect::LoadSimulation(StringHash eventType, VariantMap& eventData)
{
	main_->logicStates_.Push(new SpaceSimulation(context_, main_));
}
