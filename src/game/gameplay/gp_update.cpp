#include "gp_update.h"

#include "gp_game_state.h"
#include "gp_player_state.h"
#include "game/entities/player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/rvn.h"
#include "engine/collision/cl_controller.h"
#include "engine/collision/cl_gjk.h"
#include "engine/collision/cl_types.h"
#include "engine/render/im_render.h"
#include "engine/world/scene_manager.h"
#include "engine/world/world.h"

/*	  BIG @TODO
      player collider update is a mess, I've decoupled it from the main player update call, so now
      we must update (translate) the player's AABB every time its position changes.
      Lets fix this please.  
*/


void GP_UpdatePlayerState(Player* & player, World* world)
{
	switch (player->player_state)
	{
		case PlayerState::Standing:
		{
			// compute player next position
			auto next_position = GP_PlayerStandingGetNextPosition(player);

			// TODO: Encapsulate this as a player function
			// move player forward
			player->entity_ptr->bounding_box.Translate(next_position - player->entity_ptr->position);
			player->entity_ptr->position = next_position;
			player->Update(world);

			vec3 player_btm_sphere_center = player->entity_ptr->position + vec3(0, player->radius, 0);
			vec3 contact_point = player_btm_sphere_center + -player->last_terrain_contact_normal * player->radius;
			ImDraw::AddLine(IMHASH, player_btm_sphere_center, contact_point, COLOR_YELLOW_1);

			/* Current system: Here we are looping at most twice on the:
			   "Do stepover Vtrace, Adjust player's position to terrain, check collisions" loop
			   so we can detect when we try stepping up/down into a place where the player can't
			   fit in.
			*/
			// TODO: Not sure why we are looping here actually. Once I figure out, please explain why we can't run it once.
			for (int it = 0; it < 2; it ++)
			{
				auto vtrace = CL_DoStepoverVtrace(player, world);

				// snap player to the last terrain contact point detected if its a valid stepover hit
				if (vtrace.hit && (vtrace.delta_y > 0.0004 || vtrace.delta_y < 0))
				{
					player->entity_ptr->position.y -= vtrace.delta_y;
					player->entity_ptr->bounding_box.Translate(vec3(0, -vtrace.delta_y, 0));
					player->Update(world);
				}

				// resolve collisions
				auto results = CL_TestAndResolveCollisions(player);

				// iterate on collision results
				bool collided_with_terrain = false;
				ClResults slope;
				for (int i = 0; i < results.count; i ++)
				{
					auto result = results.results[i];

					collided_with_terrain = dot(result.normal, UnitY) > 0;

					if (collided_with_terrain)
						player->last_terrain_contact_normal = result.normal;

					bool collided_with_slope = dot(result.normal, UnitY) >= SlopeMinAngle;
					if (collided_with_slope && result.entity->slidable)
						slope = result;
				}

				// if floor is no longer beneath player's feet
				if (!vtrace.hit)
				{
					float fall_momentum_intensity = player->speed < player->fall_from_edge_push_speed ? player->fall_from_edge_push_speed : player->speed;
					vec2 fall_momentum_dir = player->v_dir_historic.xz;
					vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

					player->entity_ptr->velocity = vec3(fall_momentum.x, 0, fall_momentum.y);
					GP_ChangePlayerState(player, PlayerState::Falling);
					player->Update(world, true);
					break;
				}

				// Collided with nothing or with terrain only, break
				if (results.count == 0 || (collided_with_terrain && results.count == 1))
					break;

				if (slope.collision)
				{
					PlayerStateChangeArgs args;
					args.normal = slope.normal;
					GP_ChangePlayerState(player, PlayerState::Sliding, args);
					break;
				}
			}

			// Check interactions
			if (player->want_to_grab)
			{
				GP_CheckPlayerGrabbedLedge(player, world);
				Rvn::Print("Ran check player grabbed ledge", 1000);
			}

			break;
		}


		case PlayerState::Falling:
		{
			player->entity_ptr->velocity += Rvn::frame.duration * player->gravity;
			player->entity_ptr->position += player->entity_ptr->velocity * Rvn::frame.duration;
			player->Update(world, true);

			auto results = CL_TestAndResolveCollisions(player);
			for (int i = 0; i < results.count; i ++)
			{
				auto result = results.results[i];

				// slope collision
				{
					bool collided_with_slope = dot(result.normal, UnitY) >= SlopeMinAngle;
					if (collided_with_slope && result.entity->slidable)
					{
						PlayerStateChangeArgs args;
						args.normal = result.normal;
						GP_ChangePlayerState(player, PlayerState::Sliding, args);
						return;
					}
				}

				// floor collision
				{
					bool collided_with_terrain = dot(result.normal, UnitY) > 0;
					if (collided_with_terrain)
					{
						GP_ChangePlayerState(player, PlayerState::Standing);
						return;
					}
				}

				// else
				{
					CL_WallSlidePlayer(player, result.normal);
				}
			}

			break;
		}


		case PlayerState::Jumping:
		{
			auto& v = player->entity_ptr->velocity;

			bool no_move_command = player->v_dir.x == 0 && player->v_dir.z == 0;
			if (player->jumping_upwards && !no_move_command)
			{
				if (player->speed < player->air_speed)
				{
					player->speed += player->air_delta_speed;
					v += player->v_dir * player->air_delta_speed;
				}
			}

			v += Rvn::frame.duration * player->gravity;
			player->entity_ptr->position += player->entity_ptr->velocity * Rvn::frame.duration;
			player->Update(world, true);

			auto results = CL_TestAndResolveCollisions(player);
			for (int i = 0; i < results.count; i ++)
			{
				auto result = results.results[i];

				// collision with terrain while jumping should be super rare I guess ...
				// slope collision
				{
					bool collided_with_slope = dot(result.normal, UnitY) >= SlopeMinAngle;
					if (collided_with_slope && result.entity->slidable)
					{
						PlayerStateChangeArgs args;
						args.normal = result.normal;
						GP_ChangePlayerState(player, PlayerState::Sliding, args);
						return;
					}
				}

				// floor collision 
				{
					bool collided_with_terrain = dot(result.normal, UnitY) > 0;
					if (collided_with_terrain)
					{
						GP_ChangePlayerState(player, PlayerState::Standing);
						return;
					}
				}

				// else
				{
					CL_WallSlidePlayer(player, result.normal);
				}
			}

			// @todo - need to include case that player touches inclined terrain
			//          in that case it should also stand (or fall from ledge) and not
			//          directly fall.
			if (results.count > 0)
				GP_ChangePlayerState(player, PlayerState::Falling);

			else if (player->entity_ptr->velocity.y <= 0)
				GP_ChangePlayerState(player, PlayerState::Falling);

			break;
		}


		case PlayerState::Sliding:
		{
			ImDraw::AddLine(IMHASH, player->entity_ptr->position, player->entity_ptr->position + 1.f * player->sliding_direction, COLOR_RED_2);

			player->entity_ptr->velocity = player->v_dir * player->slide_speed;

			player->entity_ptr->position += player->entity_ptr->velocity * Rvn::frame.duration;
			player->Update(world, true);


			// RESOLVE COLLISIONS AND CHECK FOR TERRAIN CONTACT
			auto results = CL_TestAndResolveCollisions(player);

			bool collided_with_terrain = false;
			for (int i = 0; i < results.count; i ++)
			{
				// iterate on collision results
				auto result = results.results[i];
				collided_with_terrain = dot(result.normal, UnitY) > 0;
				if (collided_with_terrain)
					player->last_terrain_contact_normal = result.normal;
			}

			if (collided_with_terrain)
			{
				GP_ChangePlayerState(player, PlayerState::Standing);
				break;
			}

			auto vtrace = CL_DoStepoverVtrace(player, world);
			if (!vtrace.hit)
			{
				GP_ChangePlayerState(player, PlayerState::Falling);
			}

			break;
		}
		default:
			break;
	}

}


// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions)
// {
//    /* Does a circular array of raycasts according to parameters.
//       The circle will be considered to be 'touching the ground', hence limits for stepping up or down are applied
//       from the "center" arg y component.

//       center: circle's center
//       radius: circle radius
//       orientation0: reference direction for first ray
//       angle: angle span from 0 to 360
//       subdivisions: controls how many rays to shoot
//    */

//       bool hit_terrain = false;
//       vec3 orientation = normalize(to3d_xz(orientation0));
//       float delta_angle = angle / subdivisions;
//       float current_angle = 0;
//       while(current_angle <= angle)
//       {
//          orientation = rotate(orientation, glm::radians(delta_angle), UNIT_Y);
//          vec3 ray_origin = center + orientation * radius;
//          // moves ray up a bit
//          ray_origin.y = center.y + 1;

//          vec3 color = COLOR_RED_1;
//          if(current_angle <= angle / 2)
//             color = COLOR_RED_2;

//          bool hit = GP_do_vtrace_for_terrain(ray_origin, center.y, color);
//          hit_terrain = hit_terrain || hit;

//          current_angle += delta_angle;
//       }

//       return hit_terrain;
// }


// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color = COLOR_RED_1)
// {
//    auto vtrace_ray = Ray{ vtrace_origin, -UNIT_Y };
//    RaycastTest vtrace = test_ray_against_scene(vtrace_ray);
//    if(vtrace.hit)
//    {
//       auto hitpoint = point_from_detection(vtrace_ray, vtrace);
//       // draw arrow
//       ImDraw::add_line (IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, vtrace_origin, debug_color);
//       ImDraw::add_point(IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, debug_color);

//       float delta_y = abs(terrain_baseline_height - hitpoint.y);
//       if(delta_y <= PLAYER_STEPOVER_LIMIT)
//       {
//          // disable collision with potential floor / terrain
//          CL_Ignore_Colliders.add(vtrace.entity);
//          return true;
//       }
//    }

//    return false;
// }

vec3 GP_PlayerStandingGetNextPosition(Player* player)
{
	bool no_move_command = player->v_dir.x == 0 && player->v_dir.z == 0;

	float dt = Rvn::frame.duration;

	// Updates v_dir_historic
	if (player->v_dir.x != 0 || player->v_dir.y != 0 || player->v_dir.z != 0)
	{
		player->v_dir_historic = player->v_dir;
	}
	else if (player->v_dir_historic == vec3(0))
	{
		player->v_dir_historic = normalize(ToXz(GlobalSceneInfo::Get()->views[GameCam]->front));
	}

	if (player->speed < 0.f || no_move_command)
	{
		player->speed = 0;
	}

	float& speed = player->speed;
	float d_speed = player->acceleration * dt;


	// If stopped
	if (speed > 0 && no_move_command)
	{
		speed = 0;
		d_speed = 0;
	}

	speed += d_speed;

	// TODO: Can refactor to "player->GetSpeedLimit()"
	float speed_limit = [&player]()-> float {
		if (player->dashing)
		{
			return player->dash_speed;
		}
		if (player->walking)
		{
			return player->walk_speed;
		}
		// default
		return player->run_speed;
	}();

	if (speed > speed_limit)
	{
		speed = speed_limit;
	}

	player->entity_ptr->velocity = speed * player->v_dir; // if no movement command is issued, v_dir = 0,0,

	return player->entity_ptr->position + player->entity_ptr->velocity * dt;
}


// -------------------
// > ACTION
// -------------------

void GP_CheckTriggerInteraction(Player* player, World* world)
{
	For(world->interactables.size())
	{
		auto interactable = world->interactables[i];

		//@todo: do a cylinder vs cylinder or cylinder vs aabb test here
		CollisionMesh trigger_collider = interactable->GetTriggerCollider();
		GJK_Result gjk_test = CL_RunGjk(&player->entity_ptr->collider, &trigger_collider);
		if (gjk_test.collision)
		{
			Rvn::PrintDynamic("Trigger Interaction", 1000);

			switch (interactable->type)
			{
				case EntityType_Checkpoint:
				{
					player->SetCheckpoint(interactable);
					break;
				}
				case EntityType_TimerTrigger:
				{
					GameState.StartTimer(interactable);
					break;
				}
			}
		}
	}
}

// -------------------
// > LEDGE GRABBING
// -------------------
void GP_CheckPlayerGrabbedLedge(Player* player, World* world)
{
	Ledge ledge = CL_PerformLedgeDetection(player, world);
	if (ledge.empty)
		return;
	vec3 position = CL_GetFinalPositionLedgeVaulting(player, ledge);

	PlayerStateChangeArgs args;
	args.ledge = ledge;
	args.final_position = position;
	GP_ChangePlayerState(player, PlayerState::Vaulting, args);
}

// void GP_check_player_grabbed_ledge(Player* player)
// {
//    // ledge grab y tollerance
//    const float y_tol = 0.1;
//    // half the ledge grab semicircle region angle, in degrees 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->top().y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);


//    for(int i = 0; i < RVN::entity_buffer->size; i++)
//    {
//       Entity* entity = RVN::entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
//       {
//          float edge_y = entity->position.y + entity->get_height();
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->entity_ptr->position.x, 
//             player->entity_ptr->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             ImDraw::add_mesh(IMHASH, player->entity_ptr, future_pos);
//             if(CL_test_in_mock_position(player, future_pos))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PlayerState::Grabbing, ps_args);
//             return;
//          }
//       }

//       else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
//       {
//          float edge_y = entity->position.y;
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->entity_ptr->position.x, 
//             player->entity_ptr->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          // player is not facing slope's inclined face
//          if(get_slope_normal(entity) != test.normal_vec)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             ImDraw::add_mesh(IMHASH, player->entity_ptr, future_pos);
//             if(CL_test_in_mock_position(player, future_pos, entity))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PlayerState::Grabbing, ps_args);
//             return;
//          }
//       }
//    }
// }


// -------------------
// > VAULTING
// -------------------

// bool GP_check_player_vaulting(Player* player)
// {
//    // action cone half theta 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->entity_ptr->position.y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);

//    for(int i = 0; i < RVN::entity_buffer->size; i++)
//    {
//       Entity* entity = RVN::entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
//          continue;

//       float rel_height = (entity->position.y + entity->get_height()) - player->feet().y;

//       // short platforms should be ignored since we will use navigation meshes that include them smoothly with a nav ramp 
//       // and therefore going over them do not count as 'vaulting moves'
//       if(rel_height < 0.3) // also makes sure we only get positive rel heights
//          continue;

//       if(rel_height >= player->half_height * 2)
//          continue;

//       auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//       auto test = CL_circle_vs_square(
//          player->entity_ptr->position.x,
//          player->entity_ptr->position.z,
//          player->radius + dr,
//          x0, x1, z0, z1
//       );

//       if(!test.is_collided)
//          continue;

//       float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//       float min_theta = 180 - s_theta;
//       float max_theta = 180 + s_theta;
//       if(min_theta <= theta && theta <= max_theta)
//       {
//          // checks if area above ledge is free for standing
//          vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//          // ImDraw::add_mesh(IMHASH, player->entity_ptr, future_pos);
//          if(CL_test_in_mock_position(player, future_pos))
//          {
//             RVN::print_dynamic("Vaulting failed.");
//             continue;
//          }

//          PlayerStateChangeArgs ps_args;
//          ps_args.entity = entity;
//          // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//          ps_args.final_position = future_pos;
//          ps_args.penetration = dr - test.overlap;

//          GP_change_player_state(player, PlayerState::Vaulting, ps_args);
//          return true;
//       }
//    }
//    return false;
// }


// void GP_check_player_events(Player* player)
// {
//    // Player death
//    if(player->lives <= 0)
//    {
//       RVN::rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
//       player->die();
//       return;
//    }
// }
