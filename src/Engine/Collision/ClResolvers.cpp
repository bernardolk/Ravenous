#include "engine/collision/ClResolvers.h"
#include "engine/rvn.h"
#include "engine/collision/primitives/ray.h"
#include <glm/gtx/quaternion.hpp>

#include "ClController.h"
#include "game/entities/EPlayer.h"
#include "engine/collision/ClTypes.h"
#include "engine/utils/colors.h"
#include "engine/render/ImRender.h"
#include "engine/collision/raycast.h"

#include "engine/utils/utils.h"
#include "engine/world/World.h"


// ---------------------
// > RESOLVE COLLISION
// ---------------------

void ClResolveCollision(RCollisionResults Results, EPlayer* Player)
{
	// unstuck player
	vec3 Offset = Results.Normal * Results.Penetration;
	Player->Position += Offset;

	// update, but don't update collider
	Player->UpdateModelMatrix();
	Player->BoundingBox.Translate(Offset);

}


ClVtraceResult ClDoStepoverVtrace(EPlayer* Player, RWorld* World)
{
	/* 
	   Cast a ray at player's last point of contact with terrain to look for something steppable (terrain).
	   Will cull out any results that are to be considered too high (is a wall) or too low (is a hole) considering
	   player's current height. */

	vec3 RayOrigin = Player->GetLastTerrainContactPoint() + vec3(0, 0.21, 0);
	auto DownwardRay = RRay{RayOrigin, -UnitY};
	RRaycastTest Raytest = World->Raycast(DownwardRay, RayCast_TestOnlyFromOutsideIn);

	if (!Raytest.Hit)
		return ClVtraceResult{false};

	// auto angle = dot(get_triangle_normal(raytest.t), UNIT_Y);
	// std::cout << "Angle is: " << to_string(angle) << "\n";
	// if(angle < 1 - 0.866)
	//    return CL_VtraceResult{ false };


	// draw arrow
	auto Hitpoint = CL_GetPointFromDetection(DownwardRay, Raytest);
	RImDraw::AddLine(IMHASH, Hitpoint, RayOrigin, 1.0, true, COLOR_GREEN_1);
	RImDraw::AddPoint(IMHASH, Hitpoint, 1.0, true, COLOR_GREEN_3);

	if (abs(Player->Position.y - Hitpoint.y) <= PlayerStepoverLimit)
		return ClVtraceResult{true, Player->GetLastTerrainContactPoint().y - Hitpoint.y, Raytest.Entity};

	return ClVtraceResult{false};
}


bool GpSimulatePlayerCollisionInFallingTrajectory(EPlayer* Player, vec2 XzVelocity)
{
	/*    
	   Simulates how it would be if player fell following the xz_velocity vector.
	   If player can get in a position where he is not stuck, we allow him to fall. */

	// configs
	float DFrame = 0.014;

	auto Pos0 = Player->Position;
	auto Velocity = vec3(XzVelocity.x, 0, XzVelocity.y);

	float MaxIterations = 120;

	RImDraw::AddPoint(IMHASH, Player->Position, 2.0, false, COLOR_GREEN_1, 1);

	int Iteration = 0;
	while (true)
	{
		Velocity += DFrame * Player->Gravity;
		Player->Position += Velocity * DFrame;
		RImDraw::AddPoint(IM_ITERHASH(Iteration), Player->Position, 2.0, true, COLOR_GREEN_1, 1);

		Player->Update();

		bool Collided = ClRunTestsForFallSimulation(Player);
		if (!Collided)
			break;

		Iteration++;
		if (Iteration == MaxIterations)
		{
			// if entered here, then we couldn't unstuck the player in max_iterations * d_frame seconds of falling towards
			// player movement direction, so he can't fall there
			Player->Position = Pos0;
			Player->Update();
			return false;
		}
	}

	Player->Position = Pos0;
	Player->Update();
	return true;
}

// ---------------------
// > WALL SLIDE PLAYER
// ---------------------
void ClWallSlidePlayer(EPlayer* Player, vec3 WallNormal)
{
	// changes player velocity to be facing a wall parallel and dampens his speed
	auto& PlayerVelocity = Player->Velocity;
	if (PlayerVelocity.x == 0 && PlayerVelocity.z == 0)
		return;

	// @todo - this is not good, need to figure out a better solution for
	//       speed when hitting walls
	float WallSlideSpeedLimit = 1;
	if (Player->Velocity.length() > WallSlideSpeedLimit)
		Player->Velocity = Player->v_dir * WallSlideSpeedLimit;

	auto UpVec = vec3(0, 1, 0);
	vec3 HorizVec = Cross(UpVec, WallNormal);

	PlayerVelocity = dot(PlayerVelocity, HorizVec) * glm::normalize(HorizVec) * Player->GetSpeed();
}


// --------------------------------------
// > CL_run_tests_for_fall_simulation
// --------------------------------------
bool ClRunTestsForFallSimulation(EPlayer* Player)
{
	// It basically the usual test but without collision resolving.

	for (auto& Entry : Rvn::EntityBuffer)
	{
		// TODO: here should test for bounding box collision (or any geometric first pass test) FIRST, then do the call below
		auto Result = ClTestPlayerVsEntity(Entry.entity, Player);

		if (Result.Collision)
			return true;
	}

	return false;
}
