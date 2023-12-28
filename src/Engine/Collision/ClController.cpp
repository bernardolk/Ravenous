#include <engine/core/core.h>
#include <engine/rvn.h>
#include <engine/collision/primitives/BoundingBox.h>
#include <engine/collision/ClGjk.h>
#include <engine/collision/ClEpa.h>
#include "game/entities/EPlayer.h"
#include <engine/collision/ClTypes.h>
#include <engine/collision/ClController.h >
#include "engine/world/World.h"

// ----------------------------
// > UPDATE PLAYER WORLD CELLS   
// ----------------------------

bool ClUpdatePlayerWorldCells(EPlayer* Player)
{
	/* Updates the player's world cells
	   Returns whether there were changes or not to the cell list
	   @todo - the procedures invoked here seem to do more work than necessary. Keep this in mind.
	*/

	auto UpdateCells = RWorld::Get()->UpdateEntityWorldChunk(Player);
	if (!UpdateCells.Status == CellUpdate_OK)
	{
		Log(UpdateCells.Message.c_str());
		return false;
	}

	return UpdateCells.EntityChangedCell;
}

// ------------------------------
// > COLLISION BUFFER FUNCTIONS
// ------------------------------

void ClRecomputeCollisionBufferEntities()
{
	// copies collision-check-relevant entity ptrs to a buffer
	// with metadata about the collision check for the entity

	// Clears buffer
	Rvn::EntityBuffer.clear();

	auto EntityIter = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIter())
	{
		Rvn::EntityBuffer.push_back(EntityBufferElement{Entity, false});
	}
}

void ResetCollisionBufferChecks()
{
	for (auto& Entry : Rvn::EntityBuffer)
		Entry.CollisionChecked = false;
}


void MarkEntityChecked(const EEntity* Entity)
{
	// TODO: We can do a lot better than this.
	// marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
	for (auto& Entry : Rvn::EntityBuffer)
	{
		if (Entry.Entity == Entity)
		{
			Entry.CollisionChecked = true;
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

Array<RCollisionResults, 15> ClTestAndResolveCollisions(EPlayer* Player)
{
	// iterative collision detection
	Array<RCollisionResults, 15> ResultsArray;
	auto EntityBuffer = Rvn::EntityBuffer;
	int C = -1;
	while (true)
	{
		C++;
		// places pointer back to start
		auto Result = ClTestCollisionBufferEntitites(Player, true);

		if (Result.Collision)
		{
			MarkEntityChecked(Result.Entity);
			ClResolveCollision(Result, Player);
			ResultsArray.Add(Result);
		}
		else
			break;
	}
	ResetCollisionBufferChecks();

	return ResultsArray;
}

bool ClTestCollisions(EPlayer* Player)
{
	// iterative collision detection
	bool AnyCollision = false;
	while (true)
	{
		// places pointer back to start
		auto Result = ClTestCollisionBufferEntitites(Player, true);

		if (Result.Collision)
		{
			MarkEntityChecked(Result.Entity);
			AnyCollision = true;
		}
		else
			break;
	}
	ResetCollisionBufferChecks();

	return AnyCollision;
}

// ---------------------------
// > RUN COLLISION DETECTION
// ---------------------------

RCollisionResults ClTestCollisionBufferEntitites(EPlayer* Player, bool Iterative = true)
{
	for (auto& Entry : Rvn::EntityBuffer)
	{
		EEntity* Entity = Entry.Entity;

		if (Iterative && Entry.CollisionChecked)
			continue;

		if (!Entity->BoundingBox.Test(Player->BoundingBox))
			continue;

		auto Result = ClTestPlayerVsEntity(Entity, Player);

		if (Result.Collision)
			return Result;
	}

	return {};
}

// -------------------------
// > TEST PLAYER VS ENTITY
// -------------------------
RCollisionResults ClTestPlayerVsEntity(EEntity* Entity, EPlayer* Player)
{
	auto ClResults = RCollisionResults{};
	ClResults.Entity = Entity;

	RCollisionMesh* EntityCollider = &Entity->Collider;
	RCollisionMesh* PlayerCollider = &Player->Collider;

	GjkResult BoxGjkTest = ClRunGjk(EntityCollider, PlayerCollider);

	if (BoxGjkTest.Collision)
	{
		EpaResult Epa = ClRunEpa(BoxGjkTest.Simplex, EntityCollider, PlayerCollider);

		if (Epa.Collision)
		{
			ClResults.Penetration = Epa.Penetration;
			ClResults.Normal = Epa.Direction;
			ClResults.Collision = true;
		}
	}

	return ClResults;
}
