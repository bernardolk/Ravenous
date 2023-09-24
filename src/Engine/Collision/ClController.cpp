#include <engine/core/core.h>
#include <engine/rvn.h>
#include <engine/collision/primitives/BoundingBox.h>
#include <engine/collision/ClGjk.h>
#include <engine/collision/ClEpa.h>
#include "game/entities/player.h"
#include <engine/collision/ClTypes.h>
#include <engine/collision/ClResolvers.h>
#include <engine/collision/ClController.h>
#include "engine/world/world.h"

// ----------------------------
// > UPDATE PLAYER WORLD CELLS   
// ----------------------------

bool CL_UpdatePlayerWorldCells(Player* player)
{
	/* Updates the player's world cells
	   Returns whether there were changes or not to the cell list
	   @todo - the procedures invoked here seem to do more work than necessary. Keep this in mind.
	*/

	auto update_cells = World::Get()->UpdateEntityWorldChunk(player);
	if (!update_cells.status == CellUpdate_OK)
	{
		print(update_cells.message.c_str());
		return false;
	}

	return update_cells.entity_changed_cell;
}

// ------------------------------
// > COLLISION BUFFER FUNCTIONS
// ------------------------------

void CL_RecomputeCollisionBufferEntities()
{
	// copies collision-check-relevant entity ptrs to a buffer
	// with metadata about the collision check for the entity

	// Clears buffer
	Rvn::entity_buffer.clear();
	
	auto entity_iter = World::Get()->GetEntityIterator();
    while (auto* entity = entity_iter())
    {
		Rvn::entity_buffer.push_back(EntityBufferElement{entity, false});
	}
}

void CL_ResetCollisionBufferChecks()
{
	for (auto& entry : Rvn::entity_buffer)
		entry.collision_checked = false;
}


void CL_MarkEntityChecked(const EEntity* entity)
{
	// TODO: We can do a lot better than this.
	// marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
	for (auto& entry : Rvn::entity_buffer)
	{
		if (entry.entity == entity)
		{
			entry.collision_checked = true;
			break;
		}
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

Array<ClResults, 15> CL_TestAndResolveCollisions(Player* player)
{	
	// iterative collision detection
	Array<ClResults, 15> results_array;
	auto entity_buffer = Rvn::entity_buffer;
	int c = -1;
	while (true)
	{
		c++;
		// places pointer back to start
		auto result = CL_TestCollisionBufferEntitites(player, true);

		if (result.collision)
		{
			CL_MarkEntityChecked(result.entity);
			CL_ResolveCollision(result, player);
			results_array.Add(result);
		}
		else
			break;
	}
	CL_ResetCollisionBufferChecks();

	return results_array;
}

bool CL_TestCollisions(Player* player)
{
	// iterative collision detection
	bool any_collision = false;
	while (true)
	{
		// places pointer back to start
		auto result = CL_TestCollisionBufferEntitites(player, true);

		if (result.collision)
		{
			CL_MarkEntityChecked(result.entity);
			any_collision = true;
		}
		else
			break;
	}
	CL_ResetCollisionBufferChecks();

	return any_collision;
}

// ---------------------------
// > RUN COLLISION DETECTION
// ---------------------------

ClResults CL_TestCollisionBufferEntitites(Player* player, bool iterative = true)
{
	for (auto& entry : Rvn::entity_buffer)
	{
		EEntity* entity = entry.entity;

		if (iterative && entry.collision_checked)
			continue;

		if (!entity->bounding_box.Test(player->bounding_box))
			continue;

		auto result = CL_TestPlayerVsEntity(entity, player);

		if (result.collision)
			return result;
	}

	return {};
}

// -------------------------
// > TEST PLAYER VS ENTITY
// -------------------------
ClResults CL_TestPlayerVsEntity(EEntity* entity, Player* player)
{
	ClResults cl_results;
	cl_results.entity = entity;

	CollisionMesh* entity_collider = &entity->collider;
	CollisionMesh* player_collider = &player->collider;

	GJK_Result box_gjk_test = CL_RunGjk(entity_collider, player_collider);

	if (box_gjk_test.collision)
	{
		EPA_Result epa = CL_RunEPA(box_gjk_test.simplex, entity_collider, player_collider);

		if (epa.collision)
		{
			cl_results.penetration = epa.penetration;
			cl_results.normal = epa.direction;
			cl_results.collision = true;
		}
	}

	return cl_results;
}
