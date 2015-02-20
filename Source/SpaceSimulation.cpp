/*
 * SpaceSimulation.cpp
 *
 *  Created on: Jan 12, 2015
 *      Author: practicing01
 */

#include "AnimatedModel.h"
#include "AnimationController.h"
#include "Audio.h"
#include "CollisionShape.h"
#include "CoreEvents.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "DrawableEvents.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Font.h"
#include "Graphics.h"
#include "InputEvents.h"
#include "Model.h"
#include "Node.h"
#include "Octree.h"
#include "OctreeQuery.h"
#include "PhysicsEvents.h"
#include "Quaternion.h"
#include "Renderer.h"
#include "RenderPath.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sound.h"
#include "SoundListener.h"
#include "SoundSource.h"
#include "SoundSource3D.h"
#include "StaticModel.h"
#include "Text.h"
#include "UI.h"
#include "Vector2.h"
#include "Viewport.h"

#include "Log.h"
//#include "DebugNew.h"

#include "SpaceSimulation.h"
#include "LifeTime.h"
#include "MoveTowards.h"
#include "RotateTowards.h"
#include "SceneObjectMoveTo.h"
#include "SceneObjectRotateTo.h"

SpaceSimulation::SpaceSimulation(Context* context, Urho3DPlayer* main) :
		Object(context)
{
	//Register custom logic components.
	context->RegisterFactory<LifeTime>();
	context->RegisterFactory<MoveTowards>();
	context->RegisterFactory<RotateTowards>();
	context->RegisterFactory<SceneObjectMoveTo>();
	context->RegisterFactory<SceneObjectRotateTo>();

	main_ = main;
	elapsedTime_ = 0.0f;

	//Clear previous camera.
	main_->cameraNode_->RemoveAllChildren();
	main_->cameraNode_->RemoveAllComponents();
	main_->cameraNode_->Remove();

	main_->renderer_->SetNumViewports(2);

	//Load first scene.
	File loadFile(context_,
			main_->filesystem_->GetProgramDir()
					+ "Data/Scenes/cockpitScene.xml", FILE_READ);
	main_->scene_->LoadXML(loadFile);

	main_->cameraNode_ = main_->scene_->GetChild("camera");
	main_->viewport_->SetCamera(main_->cameraNode_->GetComponent<Camera>());

	main_->renderer_->SetViewport(0, main_->viewport_);

	//Setup second scene variables.
	scene_ = new Scene(main_->GetContext());
	cameraNode_ = new Node(main_->GetContext());

	//Load second scene.
	File loadFile2(context_,
			main_->filesystem_->GetProgramDir()
					+ "Data/Scenes/spaceSimulationScene.xml", FILE_READ);
	scene_->LoadXML(loadFile2);

	cameraNode_ = scene_->GetChild("sicarius")->GetChild("camera");

	//Setup rendering magic so the second scene doesn't clear the first scene.
	SharedPtr<RenderPath> colorUnclearRenderPath = SharedPtr<RenderPath>(
			new RenderPath());
	colorUnclearRenderPath->Load(
			main_->cache_->GetResource<XMLFile>(
					"RenderPaths/BackgroundLayer.xml"));
	viewport_ = new Viewport(main_->GetContext(), scene_,
			cameraNode_->GetComponent<Camera>());
	viewport_->SetCamera(cameraNode_->GetComponent<Camera>());
	viewport_->SetRenderPath(colorUnclearRenderPath);
	main_->renderer_->SetViewport(1, viewport_);

	//The LifeTime component will delete the title and title light after 3 seconds.
	main_->scene_->GetChild("title")->AddComponent(new LifeTime(context_, 2.0f),
			0, LOCAL);
	main_->scene_->GetChild("titleLight")->AddComponent(
			new LifeTime(context_, 2.0f), 0, LOCAL);

	//Store object handles for future reference.
	camViewButt_ = main_->scene_->GetChild("viewButt");
	cockpitView_ = true;
	//cockpitView_ = false;
	//SetCamView(cockpitView_);

	leftJoyButt_ = main_->scene_->GetChild("leftJoyButt");
	rightJoyButt_ = main_->scene_->GetChild("rightJoyButt");

	leftJoy_ = main_->scene_->GetChild("leftJoy");
	rightJoy_ = main_->scene_->GetChild("rightJoy");

	sicarius_ = scene_->GetChild("sicarius");
	sicarius_->AddComponent(new MoveTowards(context_), 0, LOCAL);
	sicarius_->AddComponent(new RotateTowards(context_), 0, LOCAL);
	sicarius_->GetComponent<AnimationController>()->PlayExclusive("Models/idle.ani", 0, true, 0.0f);
	SubscribeToEvent(sicarius_, E_NODECOLLISION, HANDLER(SpaceSimulation, HandleNodeCollision));

	movingLeftJoystick_ = false;
	movingRightJoystick_ = false;

	leftJoystickRot_ = Vector3(20.0f, 70.0f, 20.0f);
	rightJoystickRot_ = Vector3(-20.0f, 100.0f, 20.0f);

    leftJoystickClamp_ = 45.0f;
    rightJoystickClamp_ = 45.0f;

	sicariusTrans_ = Vector3(0.0f, 0.0f, 0.0f);
	sicariusRot_ = sicarius_->GetRotation();
	sicariusRotV3_ = sicarius_->GetRotation().EulerAngles();

	sicariusSpeed_ = 100.0f;
	sicariusRotSpeed_ = 45.0f;

	leftJoystickThreshold_ = Vector3(20.0f, 0.0f, 10.0f);
	rightJoystickThreshold_ = Vector3(10.0f, 0.0f, 20.0f);

    leftJoystickTouchID_ = -1;
    rightJoystickTouchID_ = -1;

    //Generate projectiles now for optimzation.
    projectile_ = scene_->CreateChild("projectile");
    projectile_->AddComponent(new SceneObjectMoveTo(context_), 0, LOCAL);
    projectile_->CreateComponent<RigidBody>();
    projectile_->GetComponent<RigidBody>()->SetMass(1.0f);
    projectile_->GetComponent<RigidBody>()->SetTrigger(true);
    projectile_->GetComponent<RigidBody>()->SetKinematic(true);
    projectile_->GetComponent<RigidBody>()->SetGravityOverride(Vector3(0.0f, 0.0f, 0.0f));
    projectile_->CreateComponent<CollisionShape>();
    projectile_->GetComponent<CollisionShape>()->SetSphere(1.0f);
    projectile_->SetScale(0.5f);
    projectile_->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
    StaticModel* projectileModel = projectile_->CreateComponent<StaticModel>();
    projectileModel->SetModel(main_->cache_->GetResource<Model>("Models/laser.mdl"));
    projectileModel->ApplyMaterialList("Models/laser.txt");
    projectile_->SetEnabled(false);
    vectoria_.Push(projectile_);

    for (int x = 0; x < 1000; x++)
    {
    	vectoria_.Push( SharedPtr<Node>(projectile_->Clone(LOCAL)) );
    }

    projectileSpeed_ = 100.0f;
    projectileRate_ = 0.125f;
    projectileRange_ = 1000.0f;
    cannonRotateSpeed_ = 0.5f;

    victoria_.Push(SharedPtr<Node>(scene_->GetChild("clades")->GetChild("cannon0")) );
    victoria_.Push(SharedPtr<Node>(scene_->GetChild("clades")->GetChild("cannon1")) );
    victoria_.Push(SharedPtr<Node>(scene_->GetChild("clades")->GetChild("cannon2")) );
    victoria_.Push(SharedPtr<Node>(scene_->GetChild("clades")->GetChild("cannon3")) );

    victoria_.At(0)->AddComponent(new SceneObjectRotateTo(context_), 0, LOCAL);
    victoria_.At(1)->AddComponent(new SceneObjectRotateTo(context_), 0, LOCAL);
    victoria_.At(2)->AddComponent(new SceneObjectRotateTo(context_), 0, LOCAL);
    victoria_.At(3)->AddComponent(new SceneObjectRotateTo(context_), 0, LOCAL);

    //Get the rotations going.

	quaterOnion_ = victoria_.At(0)->GetRotation();
	victoria_.At(0)->LookAt(sicarius_->GetChild("hitNode")->GetWorldPosition());
	quaterPounder_ = victoria_.At(0)->GetRotation();
	victoria_.At(0)->SetRotation(quaterOnion_);

    victoria_.At(0)->GetComponent<SceneObjectRotateTo>()->RotateTo(quaterPounder_, cannonRotateSpeed_, true);

	quaterOnion_ = victoria_.At(1)->GetRotation();
	victoria_.At(1)->LookAt(sicarius_->GetChild("hitNode")->GetWorldPosition());
	quaterPounder_ = victoria_.At(1)->GetRotation();
	victoria_.At(1)->SetRotation(quaterOnion_);

    victoria_.At(1)->GetComponent<SceneObjectRotateTo>()->RotateTo(quaterPounder_, cannonRotateSpeed_, true);

	quaterOnion_ = victoria_.At(2)->GetRotation();
	victoria_.At(2)->LookAt(sicarius_->GetChild("hitNode")->GetWorldPosition());
	quaterPounder_ = victoria_.At(2)->GetRotation();
	victoria_.At(2)->SetRotation(quaterOnion_);

    victoria_.At(2)->GetComponent<SceneObjectRotateTo>()->RotateTo(quaterPounder_, cannonRotateSpeed_, true);

	quaterOnion_ = victoria_.At(3)->GetRotation();
	victoria_.At(3)->LookAt(sicarius_->GetChild("hitNode")->GetWorldPosition());
	quaterPounder_ = victoria_.At(3)->GetRotation();
	victoria_.At(3)->SetRotation(quaterOnion_);

    victoria_.At(3)->GetComponent<SceneObjectRotateTo>()->RotateTo(quaterPounder_, cannonRotateSpeed_, true);

    GetSubsystem<Audio>()->SetListener(sicarius_->GetChild("soundListener")->GetComponent<SoundListener>());

    fireParticle_ = scene_->GetChild("fireParticle");
    fireRotator_ = scene_->GetChild("fireRotator");

    SpawnHell();

    hellRotationRate_ = 5.0f;
    elapsedHellTime_ = 0.0f;
    hellRotationSpeed_ = 20.0f;
    hellRot_ = Vector2(0,1);

    lifeBar_ = main_->scene_->GetChild("lifeBar");

    fireJoyButt_ = main_->scene_->GetChild("fireJoyButt");

    sicariusProjectile_ = scene_->GetChild("sicariusProjectile");
    sicariusProjectile_->AddComponent(new SceneObjectMoveTo(context_), 0, LOCAL);
    sicariusProjectile_->AddComponent(new LifeTime(context_), 0, LOCAL);
    voctaria_.Push( SharedPtr<Node>(sicariusProjectile_) );

    for (int x = 0; x < 1000; x++)
    {
    	voctaria_.Push( SharedPtr<Node>(sicariusProjectile_->Clone(LOCAL)) );
    }

    sicariusElbow_ = sicarius_->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("Elbow.R")->node_;

    SubscribeToEvent(scene_->GetChild("clades"), E_NODECOLLISION, HANDLER(SpaceSimulation, HandleNodeCollision));

	score_ = 0;
	topScore_ = 0;
	text_ = new Text(context_);
	text_->SetText(
			"Shots Hit: " + String(score_) + " Most Hits: "
			+ String(topScore_));
	text_->SetFont(main_->cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf"),
			12);
	text_->SetColor(Color(1.0f, 0.5f, 0.0f));
	text_->SetPosition(0, 0);
	text_->SetHorizontalAlignment(HA_LEFT);
	text_->SetVerticalAlignment(VA_TOP);
	GetSubsystem<UI>()->GetRoot()->AddChild(text_);

	SubscribeToEvent(E_TOUCHBEGIN, HANDLER(SpaceSimulation, TouchDown));
	SubscribeToEvent(E_TOUCHMOVE, HANDLER(SpaceSimulation, TouchDrag));
	SubscribeToEvent(E_TOUCHEND, HANDLER(SpaceSimulation, TouchUp));
	SubscribeToEvent(E_UPDATE, HANDLER(SpaceSimulation, HandleUpdate));
	SubscribeToEvent(E_SCENEOBJECTMOVETOCOMPLETE, HANDLER(SpaceSimulation, OnMoveToComplete));
	SubscribeToEvent(E_SCENEOBJECTROTATETOCOMPLETE, HANDLER(SpaceSimulation, OnRotateToComplete));
	SubscribeToEvent(E_LIFETIMECOMPLETE, HANDLER(SpaceSimulation, LifeTimeComplete));
	//SubscribeToEvent(E_ANIMATIONTRIGGER, HANDLER(SpaceSimulation, HandleAnimationTrigger));
	//SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(SpaceSimulation, HandlePostRenderUpdate));

}

SpaceSimulation::~SpaceSimulation()
{
}

void SpaceSimulation::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	GetSubsystem<Renderer>()->DrawDebugGeometry(true);
	scene_->GetComponent<DebugRenderer>()->AddLine(victoria_.At(0)->GetWorldPosition(), victoria_.At(0)->GetWorldPosition() + (victoria_.At(0)->GetDirection().Normalized() * projectileRange_), Color());
	scene_->GetComponent<DebugRenderer>()->AddLine(victoria_.At(1)->GetWorldPosition(), victoria_.At(1)->GetWorldPosition() + (victoria_.At(1)->GetDirection().Normalized() * projectileRange_), Color());
	scene_->GetComponent<DebugRenderer>()->AddLine(victoria_.At(2)->GetWorldPosition(), victoria_.At(2)->GetWorldPosition() + (victoria_.At(2)->GetDirection().Normalized() * projectileRange_), Color());
	scene_->GetComponent<DebugRenderer>()->AddLine(victoria_.At(3)->GetWorldPosition(), victoria_.At(3)->GetWorldPosition() + (victoria_.At(3)->GetDirection().Normalized() * projectileRange_), Color());

}

void SpaceSimulation::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	if (main_->input_->GetKeyDown(SDLK_ESCAPE))//SDL_SCANCODE_AC_BACK))
	{
		main_->GetSubsystem<Engine>()->Exit();
	}

	//LOGERRORF("loop");
	elapsedTime_ += timeStep;

	if (elapsedTime_ >= projectileRate_)
	{
		FireCladesProjectile();
		elapsedTime_ = 0.0f;
	}

	RotateHell(timeStep);
}

void SpaceSimulation::TouchDown(StringHash eventType, VariantMap& eventData)
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
			if (results[x].node_ == camViewButt_)
			{
				//do stuffs with butt
				cockpitView_ = !cockpitView_;
				SetCamView(cockpitView_);
			}
			else if (results[x].node_ == leftJoyButt_)
			{
				//do stuffs with butt
				movingLeftJoystick_ = true;
				leftJoystickPrevPos_.x_ = eventData[P_X].GetInt();
				leftJoystickPrevPos_.y_ = eventData[P_Y].GetInt();
				leftJoystickTouchID_ = eventData[P_TOUCHID].GetInt();
			}
			else if (results[x].node_ == rightJoyButt_)
			{
				//do stuffs with butt
				movingRightJoystick_ = true;
				rightJoystickPrevPos_.x_ = eventData[P_X].GetInt();
				rightJoystickPrevPos_.y_ = eventData[P_Y].GetInt();
				rightJoystickTouchID_ = eventData[P_TOUCHID].GetInt();
			}
			else if (results[x].node_ == fireJoyButt_)
			{
				//do stuffs with butt
				SicariusFire();
			}
		}
	}

}

void SpaceSimulation::TouchDrag(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchMove;

	if (eventData[P_TOUCHID].GetInt() == leftJoystickTouchID_)
	{
		MoveLeftJoyStick(eventData[P_X].GetInt(), eventData[P_Y].GetInt(), false);
	}
	else if (eventData[P_TOUCHID].GetInt() == rightJoystickTouchID_)
	{
		MoveRightJoyStick(eventData[P_X].GetInt(), eventData[P_Y].GetInt(), false);
	}

}

void SpaceSimulation::TouchUp(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchEnd;

	if (eventData[P_TOUCHID].GetInt() == leftJoystickTouchID_)
	{
		leftJoystickTouchID_ = -1;
		MoveLeftJoyStick(0,0,true);
	}
	else if (eventData[P_TOUCHID].GetInt() == rightJoystickTouchID_)
	{
		rightJoystickTouchID_ = -1;
		MoveRightJoyStick(0,0,true);
	}

}

void SpaceSimulation::SetCamView(bool cockpitView)
{
	if (cockpitView)
	{
		cameraNode_->SetPosition(Vector3(15.0f, 13.0f, 0.0f));
	}
	else
	{
		cameraNode_->SetPosition(Vector3(-50.0f, 0.0f, 0.0f));
	}
}

void SpaceSimulation::MoveLeftJoyStick(int x, int y, bool reset)
{
	if (reset)
	{
		sicarius_->GetComponent<MoveTowards>()->Stop();
		sicariusTrans_ = Vector3::ZERO;
		leftJoystickRot_.x_ = 20.0f;
		leftJoystickRot_.y_ = 70.0f;
		leftJoystickRot_.z_ = 20.0f;
		leftJoy_->SetRotation(Quaternion(leftJoystickRot_.x_, leftJoystickRot_.y_, leftJoystickRot_.z_));
		return;
	}

	sicariusTrans_ = Vector3::ZERO;

	//Decide if the move was more along the horizontal or the vertical
	int distanceX = Abs(leftJoystickPrevPos_.x_ - x);
	int distanceY = Abs(leftJoystickPrevPos_.y_ - y);

	//Both horizontal and vertical calculated
	{
		if (leftJoystickPrevPos_.x_ < x) //right drag
		{
			leftJoystickRot_.x_ = Clamp(leftJoystickRot_.x_ + distanceX, -leftJoystickClamp_, leftJoystickClamp_);
			leftJoy_->SetRotation(Quaternion(leftJoystickRot_.x_, leftJoystickRot_.y_, leftJoystickRot_.z_));
		}
		else if (leftJoystickPrevPos_.x_ > x) //left drag
		{
			leftJoystickRot_.x_ = Clamp(leftJoystickRot_.x_ - distanceX, -leftJoystickClamp_, leftJoystickClamp_);
			leftJoy_->SetRotation(Quaternion(leftJoystickRot_.x_, leftJoystickRot_.y_, leftJoystickRot_.z_));
		}

		if (leftJoystickPrevPos_.y_ > y)//up drag
		{
			leftJoystickRot_.z_ = Clamp(leftJoystickRot_.z_ + distanceY, -leftJoystickClamp_, leftJoystickClamp_);
			leftJoy_->SetRotation(Quaternion(leftJoystickRot_.x_, leftJoystickRot_.y_, leftJoystickRot_.z_));
		}
		else if (leftJoystickPrevPos_.y_ < y)//down drag
		{
			leftJoystickRot_.z_ = Clamp(leftJoystickRot_.z_ - distanceY, -leftJoystickClamp_, leftJoystickClamp_);
			leftJoy_->SetRotation(Quaternion(leftJoystickRot_.x_, leftJoystickRot_.y_, leftJoystickRot_.z_));
		}
	}

	leftJoystickPrevPos_.x_ = x;
	leftJoystickPrevPos_.y_ = y;

	if (Abs(leftJoystickRot_.x_) > leftJoystickThreshold_.x_)
	{
		if (leftJoystickRot_.x_ > 0.0f)
		{
			sicariusTrans_ += Vector3::BACK;
		}
		else if (leftJoystickRot_.x_ < 0.0f)
		{
			sicariusTrans_ += Vector3::FORWARD;
		}
	}

	if (Abs(leftJoystickRot_.z_) > leftJoystickThreshold_.z_)
	{
		if (leftJoystickRot_.z_ > 0.0f)
		{
			sicariusTrans_ += Vector3::RIGHT;
		}
		else if (leftJoystickRot_.z_ < 0.0f)
		{
			sicariusTrans_ += Vector3::LEFT;
		}
	}

	if (sicariusTrans_.LengthSquared() > 0.0f)
	{
		sicariusTrans_.Normalize();
	}

	sicarius_->GetComponent<MoveTowards>()->MoveToward(sicariusRot_ * sicariusTrans_, sicariusSpeed_);
}

void SpaceSimulation::MoveRightJoyStick(int x, int y, bool reset)
{
	if (reset)
	{
		sicarius_->GetComponent<RotateTowards>()->Stop();
		sicariusRot_ = sicarius_->GetRotation();
		rightJoystickRot_.x_ = -20.0f;
		rightJoystickRot_.y_ = 100.0f;
		rightJoystickRot_.z_ = 20.0f;
		rightJoy_->SetRotation(Quaternion(rightJoystickRot_.x_, rightJoystickRot_.y_, rightJoystickRot_.z_));
		return;
	}

	sicariusRotV3_ = Vector3::ZERO;

	//Decide if the move was more along the horizontal or the vertical
	int distanceX = Abs(rightJoystickPrevPos_.x_ - x);
	int distanceY = Abs(rightJoystickPrevPos_.y_ - y);

	//Both horizontal and vertical calculated
	{
		if (rightJoystickPrevPos_.x_ < x) //right drag
		{
			//sicariusRotV3_.y_ = 1.0f;
			rightJoystickRot_.x_ = Clamp(rightJoystickRot_.x_ + distanceX, -rightJoystickClamp_, rightJoystickClamp_);
			rightJoy_->SetRotation(Quaternion(rightJoystickRot_.x_, rightJoystickRot_.y_, rightJoystickRot_.z_));
		}
		else if (rightJoystickPrevPos_.x_ > x) //left drag
		{
			//sicariusRotV3_.y_ = -1.0f;
			rightJoystickRot_.x_ = Clamp(rightJoystickRot_.x_ - distanceX, -rightJoystickClamp_, rightJoystickClamp_);
			rightJoy_->SetRotation(Quaternion(rightJoystickRot_.x_, rightJoystickRot_.y_, rightJoystickRot_.z_));
		}

		if (rightJoystickPrevPos_.y_ > y)//up drag
		{
			//sicariusRotV3_.z_ = 1.0f;
			rightJoystickRot_.z_ = Clamp(rightJoystickRot_.z_ + distanceY, -rightJoystickClamp_, rightJoystickClamp_);
			rightJoy_->SetRotation(Quaternion(rightJoystickRot_.x_, rightJoystickRot_.y_, rightJoystickRot_.z_));
		}
		else if (rightJoystickPrevPos_.y_ < y)//down drag
		{
			//sicariusRotV3_.z_ = -1.0f;
			rightJoystickRot_.z_ = Clamp(rightJoystickRot_.z_ - distanceY, -rightJoystickClamp_, rightJoystickClamp_);
			rightJoy_->SetRotation(Quaternion(rightJoystickRot_.x_, rightJoystickRot_.y_, rightJoystickRot_.z_));
		}
	}


	if (Abs(rightJoystickRot_.x_) > rightJoystickThreshold_.x_)
	{
		if (rightJoystickRot_.x_ > 0.0f)
		{
			sicariusRotV3_.y_ = 1.0f;
		}
		else if (rightJoystickRot_.x_ < 0.0f)
		{
			sicariusRotV3_.y_ = -1.0f;
		}
	}

	if (Abs(rightJoystickRot_.z_) > rightJoystickThreshold_.z_)
	{
		if (rightJoystickRot_.z_ > 0.0f)
		{
			sicariusRotV3_.z_ = 1.0f;
		}
		else if (rightJoystickRot_.z_ < 0.0f)
		{
			sicariusRotV3_.z_ = -1.0f;
		}
	}

	sicarius_->GetComponent<RotateTowards>()->RotateToward(sicariusRotV3_, sicariusRotSpeed_);
	sicariusRot_ = sicarius_->GetRotation();

	if (movingLeftJoystick_)
	{
		sicarius_->GetComponent<MoveTowards>()->MoveToward(sicariusRot_ * sicariusTrans_, sicariusSpeed_);
	}

	rightJoystickPrevPos_.x_ = x;
	rightJoystickPrevPos_.y_ = y;
}

void SpaceSimulation::FireCladesProjectile()
{
/*
	Ray projectileRay = Ray(victoria_.At(0)->GetWorldPosition(), victoria_.At(0)->GetDirection().Normalized());

	PODVector<RayQueryResult> results;
	//RAY_TRIANGLE RAY_AABB
	RayOctreeQuery query(results, projectileRay,RAY_OBB, M_INFINITY,
			DRAWABLE_ANY);

	scene_->GetComponent<Octree>()->Raycast(query);

	if (results.Size())
	{
		for (int x = 0; x < results.Size(); x++)
		{//LOGERRORF(results[x].node_->GetName().CString());
			if (results[x].node_ == sicarius_)
			{
				LOGINFOF("hit");
				break;
			}
		}
	}
*/
	if (!vectoria_.Empty())
	{
		//victoria_.At(0)->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/laser1.ogg"));
		SharedPtr<Node> projectile = vectoria_.Back();
		vectoria_.Pop();
		projectile->SetEnabled(true);
		projectile->SetPosition(victoria_.At(0)->GetWorldPosition());
		projectile->SetRotation(victoria_.At(0)->GetWorldRotation());
		projectile->GetComponent<SceneObjectMoveTo>()->MoveTo(projectile->GetPosition() + (projectile->GetDirection().Normalized() * projectileRange_), projectileSpeed_, false);
	}

	if (!vectoria_.Empty())
	{
		//victoria_.At(1)->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/laser1.ogg"));
		SharedPtr<Node> projectile = vectoria_.Back();
		vectoria_.Pop();
		projectile->SetEnabled(true);
		projectile->SetPosition(victoria_.At(1)->GetWorldPosition());
		projectile->SetRotation(victoria_.At(1)->GetWorldRotation());
		projectile->GetComponent<SceneObjectMoveTo>()->MoveTo(projectile->GetPosition() + (projectile->GetDirection().Normalized() * projectileRange_), projectileSpeed_, false);
	}

	if (!vectoria_.Empty())
	{
		//victoria_.At(2)->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/laser1.ogg"));
		SharedPtr<Node> projectile = vectoria_.Back();
		vectoria_.Pop();
		projectile->SetEnabled(true);
		projectile->SetPosition(victoria_.At(2)->GetWorldPosition());
		projectile->SetRotation(victoria_.At(2)->GetWorldRotation());
		projectile->GetComponent<SceneObjectMoveTo>()->MoveTo(projectile->GetPosition() + (projectile->GetDirection().Normalized() * projectileRange_), projectileSpeed_, false);
	}

	if (!vectoria_.Empty())
	{
		//victoria_.At(3)->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/laser1.ogg"));
		SharedPtr<Node> projectile = vectoria_.Back();
		vectoria_.Pop();
		projectile->SetEnabled(true);
		projectile->SetPosition(victoria_.At(3)->GetWorldPosition());
		projectile->SetRotation(victoria_.At(3)->GetWorldRotation());
		projectile->GetComponent<SceneObjectMoveTo>()->MoveTo(projectile->GetPosition() + (projectile->GetDirection().Normalized() * projectileRange_), projectileSpeed_, false);
	}
}

void SpaceSimulation::OnMoveToComplete(StringHash eventType, VariantMap& eventData)
{
	using namespace SceneObjectMoveToComplete;

	SharedPtr<Node> projectile = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	if (projectile->GetName() == "sicariusProjectile")
	{
		projectile->SetEnabled(false);
		voctaria_.Push(projectile);
		//projectile->GetComponent<LifeTime>()->setLifeTime(0.0f);
		return;
	}

	projectile->SetEnabled(false);
	vectoria_.Push(projectile);
}

void SpaceSimulation::OnRotateToComplete(StringHash eventType, VariantMap& eventData)
{
	using namespace SceneObjectRotateToComplete;

	SharedPtr<Node> cannon = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	quaterOnion_ = cannon->GetRotation();
	cannon->LookAt(sicarius_->GetChild("hitNode")->GetWorldPosition());
	quaterPounder_ = cannon->GetRotation();
	cannon->SetRotation(quaterOnion_);

	cannon->GetComponent<SceneObjectRotateTo>()->RotateTo(quaterPounder_, cannonRotateSpeed_, true);
}

void SpaceSimulation::SpawnHell()
{
	for (float z = -2.5f; z < 2.5f; z++)
	{
		for (float y = -2.5f; y < 2.5f; y++)
		{
			for (float x = -2.5f; x < 2.5f; x++)
			{
				SharedPtr<Node> fireClone = SharedPtr<Node>(fireParticle_->Clone(LOCAL));
				fireClone->SetEnabled(true);
				fireRotator_->AddChild(fireClone);
				fireClone->SetPosition(Vector3(x * 200.0f, y * 200.0f, z * 200.0f));
				vacteria_.Push(fireClone);
			}
		}
	}
}

void SpaceSimulation::RotateHell(float timeStep)
{
	elapsedHellTime_ += timeStep;

	if (elapsedHellTime_ >= hellRotationRate_)
	{
		float tmp = hellRot_.y_;
		hellRot_.y_ = hellRot_.x_;
		hellRot_.x_ = tmp;
		elapsedHellTime_ = 0.0f;
	}

	fireRotator_->Rotate(Quaternion(hellRot_.x_ * (hellRotationSpeed_ * timeStep), hellRotationSpeed_ * timeStep, hellRot_.y_ * (hellRotationSpeed_ * timeStep)));
	return;
	SharedPtr<Node> fireClone;
	for (int x = 0; x < vacteria_.Size(); x++)
	{
		fireClone = vacteria_[x];
		//fireClone->
	}
}

void SpaceSimulation::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollision;

	Node* noed = static_cast<RigidBody*>(eventData[P_BODY].GetPtr())->GetNode();
	Node* OtherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (noed->GetName() == "clades")
	{
		if (OtherNode->GetName() == "sicariusProjectile")
		{
			OtherNode->SetEnabled(false);
			voctaria_.Push(SharedPtr<Node>(OtherNode));
			/*
			OtherNode->GetComponent<RigidBody>()->SetEnabled(false);
			OtherNode->GetComponent<CollisionShape>()->SetEnabled(false);
			OtherNode->GetComponent<LifeTime>()->setLifeTime(0.0f);
			*/
			score_++;
			UpdateScore();
		}
		return;
	}
	else if (OtherNode->GetName() == "sicariusProjectile")
	{
		return;
	}

	float ation = sicarius_->GetVar("Health").GetFloat();

	ation--;

	if (ation <= 0.0f)
	{
		ation = 10.0f;
	}

	sicarius_->SetVar("Health", ation);

	lifeBar_->SetScale(Vector3(1.0f, ation, 1.0f));

	score_ = 0;
	UpdateScore();
}

void SpaceSimulation::SicariusFire()
{
	if (voctaria_.Size())
	{
		SharedPtr<Node> projectile = voctaria_.Back();
		voctaria_.Pop();
		projectile->SetEnabled(true);
		projectile->SetPosition(sicarius_->GetWorldPosition());
		projectile->SetRotation(cameraNode_->GetWorldRotation());
		projectile->GetComponent<SceneObjectMoveTo>()->MoveTo(projectile->GetPosition() + (projectile->GetDirection().Normalized() * projectileRange_), projectileSpeed_, false);
	}

	return;
	//Dumb hack otherwise it segfaults (unless we don't SetCamView() at start).
	if (main_->scene_->GetChild("title") || main_->scene_->GetChild("titleLight"))
	{
		return;
	}
	sicarius_->GetComponent<AnimationController>()->PlayExclusive(
					"Models/shoot.ani", 0, false, 0.0f);

	Node* clown = sicariusProjectile_->Clone(LOCAL);
	clown->SetEnabled(true);
	clown->SetWorldPosition(sicariusElbow_->GetWorldPosition());
	clown->SetWorldDirection(cameraNode_->GetWorldDirection());
	clown->GetComponent<SceneObjectMoveTo>()->MoveTo(clown->GetWorldPosition() + (clown->GetWorldDirection().Normalized() * projectileRange_), projectileSpeed_, false);
}

void SpaceSimulation::LifeTimeComplete(StringHash eventType, VariantMap& eventData)
{
	SharedPtr<Node> object = SharedPtr<Node>(static_cast<Node*>(eventData[LifeTimeComplete::P_NODE].GetPtr()));

	object->RemoveAllChildren();
	object->RemoveAllComponents();
	object->Remove();
}

void SpaceSimulation::HandleAnimationTrigger(StringHash eventType, VariantMap& eventData)
{
	Node* object =static_cast<Node*>(eventData[AnimationTrigger::P_NODE].GetPtr());

	if (eventData[AnimationTrigger::P_NAME].GetString() == "shoot")
	{
		if (eventData[AnimationTrigger::P_DATA].GetString() == "shootEnd")
		{
			/*Node* clown = sicariusProjectile_->Clone(LOCAL);
			clown->SetEnabled(true);
			clown->SetWorldPosition(sicariusElbow_->GetWorldPosition());
			clown->SetWorldDirection(cameraNode_->GetWorldDirection());
			clown->GetComponent<SceneObjectMoveTo>()->MoveTo(clown->GetWorldPosition() + (clown->GetWorldDirection().Normalized() * projectileRange_), projectileSpeed_, false);
			*/sicarius_->GetComponent<AnimationController>()->PlayExclusive(
					"Models/idle.ani", 0, true, 0.0f);
		}
	}
}

void SpaceSimulation::UpdateScore()
{
	if (score_ > topScore_)
	{
		topScore_ = score_;
	}
	text_->SetText(
			"Shots Hit: " + String(score_) + " Most Hits: "
			+ String(topScore_));
}
