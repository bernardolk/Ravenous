#include <engine/collision/cl_resolvers.h>
#include <engine/rvn.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/ray.h>
#include <glm/gtx/quaternion.hpp>
#include "engine/entities/entity.h"
#include "game/entities/player.h"
#include <engine/world/world.h>
#include <engine/collision/cl_types.h>
#include "engine/utils/colors.h"
#include <engine/render/im_render.h>
#include <engine/collision/raycast.h>

#include "cl_controller.h"
#include "engine/utils/utils.h"


// ---------------------
// > RESOLVE COLLISION
// ---------------------

void CL_ResolveCollision(ClResults results, Player* player)
{
	// unstuck player
	vec3 offset = results.normal * results.penetration;
	player->position += offset;

	// update, but don't update collider
	player->UpdateModelMatrix();
	player->bounding_box.Translate(offset);

}


ClVtraceResult CL_DoStepoverVtrace(Player* player, T_World* world)
{
	/* 
	   Cast a ray at player's last point of contact with terrain to look for something steppable (terrain).
	   Will cull out any results that are to be considered too high (is a wall) or too low (is a hole) considering
	   player's current height. */

	vec3 ray_origin = player->GetLastTerrainContactPoint() + vec3(0, 0.21, 0);
	auto downward_ray = Ray{ray_origin, -UnitY};
	RaycastTest raytest = world->Raycast(downward_ray, RayCast_TestOnlyFromOutsideIn, player);

	if (!raytest.hit)
		return ClVtraceResult{false};

	// auto angle = dot(get_triangle_normal(raytest.t), UNIT_Y);
	// std::cout << "Angle is: " << to_string(angle) << "\n";
	// if(angle < 1 - 0.866)
	//    return CL_VtraceResult{ false };


	// draw arrow
	auto hitpoint = CL_GetPointFromDetection(downward_ray, raytest);
	ImDraw::AddLine(IMHASH, hitpoint, ray_origin, 1.0, true, COLOR_GREEN_1);
	ImDraw::AddPoint(IMHASH, hitpoint, 1.0, true, COLOR_GREEN_3);

	if (abs(player->position.y - hitpoint.y) <= PlayerStepoverLimit)
		return ClVtraceResult{true, player->GetLastTerrainContactPoint().y - hitpoint.y, raytest.entity};

	return ClVtraceResult{false};
}


bool GP_SimulatePlayerCollisionInFallingTrajectory(Player* player, vec2 xz_velocity)
{
	/*    
	   Simulates how it would be if player fell following the xz_velocity vector.
	   If player can get in a position where he is not stuck, we allow him to fall. */

	// configs
	float d_frame = 0.014;

	auto pos_0 = player->position;
	auto vel = vec3(xz_velocity.x, 0, xz_velocity.y);

	float max_iterations = 120;

	ImDraw::AddPoint(IMHASH, player->position, 2.0, false, COLOR_GREEN_1, 1);

	auto* world = T_World::Get();
	
	int iteration = 0;
	while (true)
	{
		vel += d_frame * player->gravity;
		player->position += vel * d_frame;
		ImDraw::AddPoint(IM_ITERHASH(iteration), player->position, 2.0, true, COLOR_GREEN_1, 1);

		player->Update(world);

		bool collided = CL_RunTestsForFallSimulation(player);
		if (!collided)
			break;

		iteration++;
		if (iteration == max_iterations)
		{
			// if entered here, then we couldn't unstuck the player in max_iterations * d_frame seconds of falling towards
			// player movement direction, so he can't fall there
			player->position = pos_0;
			player->Update(world);
			return false;
		}
	}

	player->position = pos_0;
	player->Update(world);
	return true;
}

// ---------------------
// > WALL SLIDE PLAYER
// ---------------------
void CL_WallSlidePlayer(Player* player, vec3 wall_normal)
{
	// changes player velocity to be facing a wall parallel and dampens his speed
	auto& pv = player->velocity;
	if (pv.x == 0 && pv.z == 0)
		return;

	// @todo - this is not good, need to figure out a better solution for
	//       speed when hitting walls
	float wall_slide_speed_limit = 1;
	if (player->speed > wall_slide_speed_limit)
		player->speed = wall_slide_speed_limit;

	auto up_vec = vec3(0, 1, 0);
	vec3 horiz_vec = Cross(up_vec, wall_normal);

	pv = dot(pv, horiz_vec) * normalize(horiz_vec) * player->speed;
}


// --------------------------------------
// > CL_run_tests_for_fall_simulation
// --------------------------------------
bool CL_RunTestsForFallSimulation(Player* player)
{
	// It basically the usual test but without collision resolving.

	auto entity_buffer = Rvn::entity_buffer;
	auto buffer = entity_buffer->buffer;
	auto entity_list_size = entity_buffer->size;
	bool terrain_collision = false;

	for (int i = 0; i < entity_list_size; i++)
	{
		E_Entity* entity = buffer->entity;

		// TODO: here should test for bounding box collision (or any geometric first pass test) FIRST, then do the call below
		auto result = CL_TestPlayerVsEntity(entity, player);

		if (result.collision)
		{
			return true;
		}

		buffer++;
	}

	return false;
}
