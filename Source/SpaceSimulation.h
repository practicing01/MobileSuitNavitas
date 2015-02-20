/*
 * SpaceSimulation.h
 *
 *  Created on: Jan 12, 2015
 *      Author: practicing01
 */

#pragma once

#include "Object.h"
#include "Urho3DPlayer.h"
#include "Text.h"

using namespace Urho3D;

class SpaceSimulation : public Object
{
	OBJECT(SpaceSimulation);
public:
	SpaceSimulation(Context* context, Urho3DPlayer* main);
	~SpaceSimulation();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
	void TouchDown(StringHash eventType, VariantMap& eventData);
	void TouchUp(StringHash eventType, VariantMap& eventData);
	void TouchDrag(StringHash eventType, VariantMap& eventData);
	void SetCamView(bool cockpitView);
	void MoveLeftJoyStick(int x, int y, bool reset);
	void MoveRightJoyStick(int x, int y, bool reset);
	void FireCladesProjectile();
	void OnMoveToComplete(StringHash eventType, VariantMap& eventData);
	void OnRotateToComplete(StringHash eventType, VariantMap& eventData);
	void SpawnHell();
	void RotateHell(float timeStep);
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	void SicariusFire();
	void LifeTimeComplete(StringHash eventType, VariantMap& eventData);
	void HandleAnimationTrigger(StringHash eventType, VariantMap& eventData);
	void UpdateScore();

	Urho3DPlayer* main_;
	float elapsedTime_;

    Quaternion sicariusRot_;
    Quaternion quaterOnion_;
    Quaternion quaterPounder_;
	SharedPtr<Viewport> viewport_;
	SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> camViewButt_;
    SharedPtr<Node> leftJoyButt_;
    SharedPtr<Node> rightJoyButt_;
    SharedPtr<Node> leftJoy_;
    SharedPtr<Node> rightJoy_;
    SharedPtr<Node> sicarius_;
    SharedPtr<Node> projectile_;
    SharedPtr<Node> fireParticle_;
    SharedPtr<Node> fireRotator_;
    SharedPtr<Node> lifeBar_;
    SharedPtr<Node> fireJoyButt_;
    SharedPtr<Node> sicariusProjectile_;
    SharedPtr<Node> sicariusElbow_;
	SharedPtr<Text> text_;
    Vector<SharedPtr<Node> > vectoria_;
    Vector<SharedPtr<Node> > victoria_;
    Vector<SharedPtr<Node> > vacteria_;
    Vector<SharedPtr<Node> > voctaria_;
    Vector2 leftJoystickPrevPos_;
    Vector2 rightJoystickPrevPos_;
    Vector2 hellRot_;
    Vector3 leftJoystickRot_;
    Vector3 rightJoystickRot_;
    Vector3 sicariusTrans_;
    Vector3 sicariusRotV3_;
    Vector3 leftJoystickThreshold_;
    Vector3 rightJoystickThreshold_;
    Vector3 vivi_;
    bool cockpitView_;
    bool movingLeftJoystick_;
    bool movingRightJoystick_;
    float sicariusSpeed_;
    float sicariusRotSpeed_;
    float leftJoystickClamp_;
    float rightJoystickClamp_;
    float projectileSpeed_;
    float projectileRate_;
    float projectileRange_;
    float cannonRotateSpeed_;
    float hellRotationRate_;
    float elapsedHellTime_;
    float hellRotationSpeed_;
    int leftJoystickTouchID_;
    int rightJoystickTouchID_;
	int score_;
	int topScore_;
};
