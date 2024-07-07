#include "engine/core/core.h"
#include "game/animation/AnPlayer.h"
#include "..\Entities\Player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/rvn.h"

const map<RPlayerAnimationState, float> PlayerAnimationDurations =
{
	{RPlayerAnimationState::Jumping, 400},
	{RPlayerAnimationState::Landing, 200},
	{RPlayerAnimationState::LandingFall, 400},
	{RPlayerAnimationState::Vaulting, 0}
};

void AnAnimatePlayer(EPlayer* Player)
{
	if (Player->AnimState == RPlayerAnimationState::NoAnimation)
		return;

	auto& Frame = RavenousEngine::GetFrame();

	// updates animation run time
	Player->AnimT += Frame.Duration * 1000;

	// check if animation is completed
	bool EndAnim = false;

	auto* FindDuration = Find(PlayerAnimationDurations, Player->AnimState);
	if (!FindDuration)
		return;

	float AnimDuration = *FindDuration;
	if (AnimDuration > 0 && Player->AnimT >= AnimDuration)
	{
		Player->AnimT = AnimDuration;
		EndAnim = true;
	}

	// dispatch call to correct update function depending on Player animation state
	bool Interrupt = false;
	switch (Player->AnimState)
	{
		case RPlayerAnimationState::Jumping:
		{
			Interrupt = AnUpdatePlayerJumpingAnimation(Player);
			break;
		}

		case RPlayerAnimationState::Landing:
		{
			Interrupt = AnUpdatePlayerLandingAnimation(Player);
			break;
		}

		case RPlayerAnimationState::LandingFall:
		{
			Interrupt = AnUpdatePlayerLandingFallAnimation(Player);
			break;
		}

		case RPlayerAnimationState::Vaulting:
		{
			Interrupt = AnPlayerVaulting(Player);
			{
				if (Interrupt)
				{
					Player->ChangeStateTo(NPlayerState::Standing);
				}
				break;
			}
		}

		default:
			break;
	}

	// stop animation if completed or interrupted
	if (EndAnim || Interrupt)
	{
		Player->AnimState = RPlayerAnimationState::NoAnimation;
		Player->AnimT = 0;
	}
}


bool AnUpdatePlayerJumpingAnimation(EPlayer* Player)
{
	// // interpolate between 0 and duration the Player's height
	// float anim_d            = PLAYER_ANIMATION_DURATIONS[PlayerAnimationState::Jumping];
	// // float new_half_height   = Player->height - 0.1 * Player->anim_t / anim_d;
	// float h_diff            = Player->half_height - new_half_height;

	// // Player->half_height = new_half_height;

	// // @todo: should modify collider here
	// // Player->collision_geometry.cylinder.half_length = new_half_height;
	// Player->scale.y -= h_diff;
	// // compensates Player shrinkage so he appears to be lifting the legs up
	// Player->position.y += h_diff * 2;

	return false;
}


bool AnUpdatePlayerLandingAnimation(EPlayer* Player)
{
	// bool interrupt = false;
	// // add a linear height step of 0.5m per second
	// float a_step = 0.5 * RVN::frame.duration; 
	// float new_half_height = Player->half_height + a_step;
	// if(new_half_height >= Player->height)
	// {
	//    new_half_height = Player->height;
	//    a_step = Player->height - Player->scale.y;
	//    interrupt = true;
	// }

	// Player->half_height = new_half_height;

	// // @todo: should modify collider here
	// // Player->collision_geometry.cylinder.half_length = new_half_height;
	// Player->scale.y += a_step;

	// return interrupt;
	return false;
}


bool AnUpdatePlayerLandingFallAnimation(EPlayer* Player)
{
	// float anim_d = PLAYER_ANIMATION_DURATIONS[PlayerAnimationState::LandingFall];
	// bool interrupt = false;
	// // sets the % of the duration of the animation that consists
	// // of Player bending his knees on the fall, the rest is standing up again
	// float landing_d = anim_d * 0.25;

	// // landing part
	// if(Player->anim_t <= landing_d)
	// {
	//    float new_half_height = Player->height - 0.05 * Player->anim_t / landing_d;
	//    float h_diff = Player->half_height - new_half_height;

	//    Player->half_height = new_half_height;

	//    // @todo: should modify collider here
	//    // Player->collision_geometry.cylinder.half_length = new_half_height;
	//    Player->scale.y -= h_diff;
	// }
	// // standing part
	// else if(Player->anim_t > landing_d)
	// {
	//    float a_step = 0.5 * RVN::frame.duration; 
	//    float new_half_height = Player->half_height + a_step;
	//    if(new_half_height >= Player->height)
	//    {
	//       new_half_height = Player->height;
	//       a_step = Player->height - Player->scale.y;
	//       interrupt = true;
	//    }

	//    Player->half_height = new_half_height;

	//    // @todo: should modify collider here
	//    // Player->collision_geometry.cylinder.half_length = new_half_height;
	//    Player->scale.y += a_step;
	// }

	// return interrupt;
	return false;
}


bool AnPlayerVaulting(EPlayer* Player)
{
	auto& Frame = RavenousEngine::GetFrame();

	vec3& PlayerPosition = Player->Position;

	// animation speed in m/s
	const float VY = 2.f / 1.f;
	const float VXz = 2.f / 2.f;

	vec3 AnimTrajectory = Player->AnimFinalPos - Player->AnimOrigPos;

	vec3 Dist = Player->AnimFinalPos - PlayerPosition;
	auto DistSign = vec3(Sign(Dist.x), Sign(Dist.y), Sign(Dist.z));
	auto DeltaPosition = vec3(VXz * Frame.Duration, VY * Frame.Duration, VXz * Frame.Duration);

	// updates Player position
	for (int i = 0; i < 3; i++)
	{
		// I feel like the sign here is unnecessary if we have a anim_direction set
		if (abs(Dist[i]) >= DeltaPosition[i] && Sign(AnimTrajectory[i]) == DistSign[i])
			PlayerPosition[i] += DistSign[i] * DeltaPosition[i];
		else
			PlayerPosition[i] = Player->AnimFinalPos[i];
	}


	// camera direction animation
	if (!Player->AnimFinishedTurning)
	{
		auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

		float OrigSva = VectorAngleSigned(normalize(static_cast<vec2>(Player->AnimOrigDir.xz)), Player->AnimFinalDir.xz);
		float OrigAngle = glm::degrees(OrigSva);
		float OrigSign = Sign(OrigAngle);
		float TurnAngle = 0.5 * OrigSign;
		RCameraManager::ChangeCameraDirection(PlayerCamera, TurnAngle, 0.f);

		float UpdatedSva = VectorAngleSigned(normalize(static_cast<vec2>(PlayerCamera->Front.xz)), Player->AnimFinalDir.xz);
		float UpdatedAngle = glm::degrees(UpdatedSva);
		float UpdatedSign = Sign(UpdatedAngle);
		if (UpdatedSign != OrigSign)
		{
			RCameraManager::ChangeCameraDirection(PlayerCamera, -1.0 * UpdatedAngle, 0.f);
			Player->AnimFinishedTurning = true;
		}
	}

	/*
	RVN::print_dynamic("front: " + to_string(normalize(to2d_xz(pCam->Front))));
	RVN::print_dynamic("final dir: " + to_string(Player->anim_final_dir), 0, vec3(0.8, 0.8, 0.8));
	RVN::print_dynamic("orig angle: " + to_string(orig_angle), 0, vec3(0.8, 0.8, 0.8));
	RVN::print_dynamic("current angle: " +  to_string(updated_angle));
	RVN::print_dynamic("sva cam-final: " +  to_string(UpdatedSva), 0, vec3(0,0.8,0.1));
	RVN::print_dynamic("sva orig-final: " +  to_string(OrigSva), 0, vec3(0,0.8,0.1));
	RVN::print_dynamic("orig sign: " +  to_string(orig_sign), 0, vec3(0.8,0.0,0.1));
	RVN::print_dynamic("updated sign: " +  to_string(updated_sign), 0, vec3(0.8,0.0,0.1));
	*/

	if (IsEqual(PlayerPosition, Player->AnimFinalPos) && Player->AnimFinishedTurning)
	{
		return true;
	}
	return false;
}


void ForceInterruptPlayerAnimation(EPlayer* Player)
{
	// Player->anim_state = PlayerAnimationState::NoAnimation;
	// Player->anim_t = 0;
	// // Player->half_height = Player->height;
	// Player->scale.y = Player->height;

	// // @todo: should modify collider here
	// // Player->collision_geometry.cylinder.half_length = Player->height;
}
