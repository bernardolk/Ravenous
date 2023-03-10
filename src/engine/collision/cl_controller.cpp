#include <string>
#include <vector>
#include <engine/core/types.h>
#include <engine/rvn.h>
//#include <engine/camera.h>
#include <engine/vertex.h>
#include <engine/collision/primitives/bounding_box.h>
#include <map>
#include <iostream>
#include <engine/mesh.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/collision/collision_mesh.h>
#include "engine/entities/entity.h"
//#include <engine/collision/primitives/ray.h>
#include <engine/world/world.h>
#include <engine/rvn.h>
#include <engine/collision/simplex.h>
#include <engine/collision/cl_gjk.h>
#include <engine/collision/cl_epa.h>
#include <player.h>
#include <colors.h>
#include <engine/collision/cl_types.h>
#include <engine/collision/cl_resolvers.h>
#include <engine/collision/cl_controller.h>
// #include <cl_log.h>
#include <chrono>

// ----------------------------
// > UPDATE PLAYER WORLD CELLS   
// ----------------------------

bool CL_update_player_world_cells(Player* player, World* world)
{
	/* Updates the player's world cells
	   Returns whether there were changes or not to the cell list
	   @todo - the procedures invoked here seem to do more work than necessary. Keep this in mind.
	*/

	auto update_cells = world->UpdateEntityWorldCells(player->entity_ptr);
	if(!update_cells.status == CellUpdate_OK)
	{
		std::cout << update_cells.message << "\n";
		return false;
	}

	return update_cells.entity_changed_cell;
	std::cout << " CBUFFER: " << Rvn::rm_buffer;
}


// ------------------------------
// > COLLISION BUFFER FUNCTIONS
// ------------------------------

void CL_recompute_collision_buffer_entities(Player* player)
{
	// copies collision-check-relevant entity ptrs to a buffer
	// with metadata about the collision check for the entity
	auto collision_buffer = Rvn::entity_buffer->buffer;
	int entity_count = 0;
	for(int i = 0; i < player->entity_ptr->world_cells_count; i++)
	{
		auto cell = player->entity_ptr->world_cells[i];
		for(int j = 0; j < cell->count; j++)
		{
			auto entity = cell->entities[j];

			// adds to buffer only if not present already
			bool present = false;
			for(int k = 0; k < entity_count; k++)
				if(collision_buffer[k].entity->id == entity->id)
				{
					present = true;
					break;
				}

			if(!present)
			{
				collision_buffer[entity_count].entity = entity;
				collision_buffer[entity_count].collision_check = false;
				entity_count++;
				if(entity_count > Rvn::collision_buffer_capacity)
					assert(false);
			}
		}
	}

	Rvn::entity_buffer->size = entity_count;
}


void CL_reset_collision_buffer_checks()
{
	for(int i = 0; i < Rvn::entity_buffer->size; i++)
		Rvn::entity_buffer->buffer[i].collision_check = false;
}


void CL_mark_entity_checked(Entity* entity)
{
	// marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
	auto entity_buffer = Rvn::entity_buffer;
	auto entity_element = entity_buffer->buffer;
	for(int i = 0; i < entity_buffer->size; ++i)
	{
		if(entity_element->entity == entity)
		{
			entity_element->collision_check = true;
			break;
		}
		entity_element++;
	}
}

// --------------------------------------
// > RUN ITERATIVE COLLISION DETECTION
// --------------------------------------
/* Current strategy looks like this:
   - We have a collision buffer which holds all entities currently residing inside player's current world cells.
   - We iterate over these entities and test them one by one, if we encounter a collision, we resolve it and
      mark that entity as checked for this run.
   - Player state is changed accordingly, in this step. Not sure if that is a good idea or not. Probably not.
      Would be simpler to just unstuck player and update and then change player state as a final step.
   - We then run the tests again, so to find new collisions at the player's new position.
   - Once we don't have more collisions, we stop checking.
*/

ClResultsArray CL_test_and_resolve_collisions(Player* player)
{
	// iterative collision detection
	auto results_array = ClResultsArray();
	auto entity_buffer = Rvn::entity_buffer;
	int c = -1;
	while(true)
	{
		c++;
		// places pointer back to start
		auto buffer = entity_buffer->buffer;
		auto result = CL_test_collision_buffer_entitites(player, buffer, entity_buffer->size, true);

		if(result.collision)
		{
			CL_mark_entity_checked(result.entity);
			CL_ResolveCollision(result, player);
			results_array.results[results_array.count] = result;
			results_array.count++;
		}
		else
			break;
	}
	CL_reset_collision_buffer_checks();

	return results_array;
}


bool CL_test_collisions(Player* player)
{
	// iterative collision detection
	bool any_collision = false;
	auto entity_buffer = Rvn::entity_buffer;
	while(true)
	{
		// places pointer back to start
		auto buffer = entity_buffer->buffer;
		auto result = CL_test_collision_buffer_entitites(player, buffer, entity_buffer->size, true);

		if(result.collision)
		{
			CL_mark_entity_checked(result.entity);
			any_collision = true;
		}
		else
			break;
	}
	CL_reset_collision_buffer_checks();

	return any_collision;
}

// ---------------------------
// > RUN COLLISION DETECTION
// ---------------------------

ClResults CL_test_collision_buffer_entitites(
	Player* player,
	EntityBufferElement* buffer,
	int entity_list_size,
	bool iterative = true)
{

	bool test = false;
	for(int i = 0; i < entity_list_size; i++)
	{
		Entity* entity = buffer[i].entity;

		bool entity_is_player = entity->name == "Player";
		bool checked = iterative && buffer->collision_check;

		if(entity_is_player || checked)
			continue;

		if(!entity->bounding_box.Test(player->entity_ptr->bounding_box))
			continue;
		if(!test)
		{
			player->entity_ptr->UpdateCollider();
			test = true;
		}

		auto result = CL_test_player_vs_entity(entity, player);

		if(result.collision)
			return result;
	}

	return {};
}

// -------------------------
// > TEST PLAYER VS ENTITY
// -------------------------
ClResults CL_test_player_vs_entity(Entity* entity, Player* player)
{
	using micro = std::chrono::microseconds;

	ClResults cl_results;
	cl_results.entity = entity;

	Entity* player_entity = player->entity_ptr;

	CollisionMesh* entity_collider = &entity->collider;
	CollisionMesh* player_collider = &player_entity->collider;

	// auto start = std::chrono::high_resolution_clock::now(); 
	GJK_Result box_gjk_test = CL_run_GJK(entity_collider, player_collider);
	// auto finish = std::chrono::high_resolution_clock::now();

	// std::cout << "CL_run_GJK() took "
	//          << std::chrono::duration_cast<micro>(finish - start).count()
	//          << " microseconds\n";

	bool b_gjk = false;
	bool b_epa = false;
	if(box_gjk_test.collision)
	{
		b_gjk = true;

		// start = std::chrono::high_resolution_clock::now(); 
		EPA_Result epa = CL_run_EPA(box_gjk_test.simplex, entity_collider, player_collider);
		// finish = std::chrono::high_resolution_clock::now();

		// std::cout << "CL_run_EPA() took "
		//    << std::chrono::duration_cast<micro>(finish - start).count()
		//    << " microseconds\n";

		if(epa.collision)
		{
			b_epa = true;
			cl_results.penetration = epa.penetration;
			cl_results.normal = epa.direction;
			cl_results.collision = true;
		}
	}

	return cl_results;
}
