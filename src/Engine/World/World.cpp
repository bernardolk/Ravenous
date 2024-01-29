#include "engine/world/World.h"
#include "engine/catalogues.h"
#include "Engine/RavenousEngine.h"
#include "engine/entities/Entity.h"
#include "engine/entities/lights.h"
#include "engine/render/ImRender.h"
#include "engine/utils/utils.h"
#include "game/entities/EPlayer.h"

RWorld::RWorld()
{
	auto& ActiveChunk = *Chunks.AddNew();
	ActiveChunks.push_back(&ActiveChunk);
	ActiveChunk.i = 0;
	ActiveChunk.j = 0;
	ActiveChunk.k = 0;

	// insert in map
	ChunksMap[ActiveChunk.GetChunkPosition()] = &ActiveChunk;
}

void RWorld::Update()
{
	UpdateTransforms();
	UpdateTraits();
}

void RWorld::Erase()
{
	for (auto& EntitySlot : EntityStorage.EntitySlots) {
		delete EntitySlot.Value;
		EntityStorage.Empty(&EntitySlot);
	}
}

void RWorld::UpdateTransforms()
{
	REntityIterator It;
	while (auto* Entity = It()) {
		Entity->Update();
	}
}

void RWorld::DeleteEntitiesMarkedForDeletion()
{
	for (auto& EntitySlot : EntitiesToDelete) {
		delete EntitySlot->Value;
		EntityStorage.Empty(EntitySlot.Get());
	}
	EntitiesToDelete.clear();
}

void RWorld::UpdateTraits()
{
	// TODO: We are taking the simplest approach now with a vector of entities instead of a type-unsafe memory arena allocator strategy.
	// One improvement futurely could be to sort the entity vectors by entity type on load if things get slow in this piece of code.
	// We could maintain a ViewList into the memory arena by having a vector<EEntity*> be updated as entities get added, and then use that list specifically for debugging only.
	// The reason why I moved away from the world chunk tech for now is that it is overkill for what the game currently is, and it makes debugging entities very hard.
	// After PoC for the game is complete, I probably will return to it to have easy loading / unloading of world chunks for streaming the world.

	auto* TraitsManager = EntityTraitsManager::Get();
	REntityIterator It;
	while(auto* Entity = It())
	{
		auto TraitsList = TraitsManager->GetEntityTraits(Entity);
		for (RTraitID TraitID : TraitsList)
		{
			auto* TraitUpdateFunc = TraitsManager->GetUpdateFunc(Entity->TypeID, TraitID);
			TraitUpdateFunc(Entity);
		}
	}
	
// Disabled while we don't care for world chunks
#if 0
	auto* TraitsManager = EntityTraitsManager::Get();
	for (RTraitID TraitId : TraitsManager->EntityTraits)
	{
		for (auto* Chunk : ActiveChunks)
		{
			Chunk->InvokeTraitUpdateOnAllTypes(TraitId);
		}
	}
#endif
}

TIterator<RWorldChunk> RWorld::GetChunkIterator()
{
	return Chunks.GetIterator();
}

REntityIterator::REntityIterator() : World(RWorld::Get())
{
	if (World->EntityStorage.EntitySlots.empty()) return;
	CurrentEntitySlot = &World->EntityStorage.EntitySlots[0];
}

EEntity* REntityIterator::operator()()
{
	auto* LastSlot = &World->EntityStorage.EntitySlots.back();
	if (CurrentEntitySlot == LastSlot + 1) return nullptr;

	while (!World->EntityStorage.SlotContainsEntity(CurrentEntitySlot)) {
		CurrentEntitySlot++;
		if (CurrentEntitySlot == LastSlot + 1) return nullptr;
	}

	auto* Entity = CurrentEntitySlot->Value;
	CurrentEntitySlot++;
	return Entity;
	
// TODO: Reactivate again when we need more headaches and world partitioning to be online.
#if 0
	EEntity* Entity = ChunkIterator();
	if (!Entity && CurrentChunkIndex < TotalActiveChunks - 1)
	{
		CurrentChunkIndex++;
		ChunkIterator = World->ActiveChunks[CurrentChunkIndex]->GetIterator();
		Entity = ChunkIterator();
	}
	return Entity;
#endif
}

EEntity* RWorldChunkEntityIterator::operator()()
{
	if (BlockIdx < Chunk->ChunkStorage.StorageMetadataArray.Num())
	{
		auto* BlockMetadata = Chunk->ChunkStorage.StorageMetadataArray.GetAt(BlockIdx);
		if (EntityIdx < BlockMetadata->EntityCount)
		{
			return reinterpret_cast<EEntity*>(BlockMetadata->DataStart + BlockMetadata->TypeSize * EntityIdx++);
		}

		EntityIdx = 0;
		BlockIdx++;
	}
	return nullptr;
}

RRaycastTest RWorld::Raycast(const RRay& Ray, const NRayCastType TestType, const EEntity* Skip, const float MaxDistance) const
{
	float MinDistance = MaxFloat;
	
	RRaycastTest ClosestHit;
	ClosestHit.Hit = false;
	ClosestHit.Distance = -1;

	REntityIterator It;
	while (auto* Entity = It())
	{
		if ((TestType == RayCast_TestOnlyVisibleEntities && Entity->Flags & EntityFlags_InvisibleEntity)
			|| (Skip != nullptr && Entity->ID == Skip->ID))
			continue;

		const auto Test = TestRayAgainstEntity(Ray, Entity, TestType);
		if (Test.Hit && Test.Distance < MinDistance && Test.Distance < MaxDistance) {
			ClosestHit = Test;
			ClosestHit.Entity = Entity;
			MinDistance = Test.Distance;
		}
	}

	return ClosestHit;
}

RRaycastTest RWorld::Raycast(const RRay& Ray, const EEntity* Skip, const float MaxDistance) const
{
	return this->Raycast(Ray, RayCast_TestOnlyFromOutsideIn, Skip);
}

RRaycastTest RWorld::LinearRaycastArray(const RRay FirstRay, int Qty, float Spacing) const
{
	/* 
	   Casts multiple Ray towards the first_Ray direction, with dir pointing upwards,
	   qty says how many Rays to shoot and spacing, well, the spacing between each Ray.
	*/

	RRay Ray = FirstRay;
	float HighestY = MinFloat;
	float ShorTestZ = MaxFloat;
	RRaycastTest BestHitResults;

	EPlayer* Player = EPlayer::Get();

	for (int i = 0; i < Qty; i++)
	{
		auto Test = this->Raycast(Ray, RayCast_TestOnlyFromOutsideIn, nullptr, Player->GrabReach);
		if (Test.Hit)
		{
			if (Test.Distance < ShorTestZ || (AreEqualFloats(Test.Distance, ShorTestZ) && HighestY < Ray.Origin.y))
			{
				HighestY = Ray.Origin.y;
				ShorTestZ = Test.Distance;
				BestHitResults = Test;
			}
		}

		RImDraw::AddLine(IM_ITERHASH(i), Ray.Origin, Ray.Origin + Ray.Direction * Player->GrabReach, 0, COLOR_GREEN_1, 1.2f, false);

		Ray = RRay{Ray.Origin + UnitY * Spacing, Ray.Direction};
	}

	if (BestHitResults.Hit)
	{
		vec3 Hitpoint = BestHitResults.GetPoint();
		RImDraw::AddPoint(IMHASH, Hitpoint, 0, COLOR_RED_1, 2.0, true);
	}

	return BestHitResults;
}

RRaycastTest RWorld::RaycastLights(const RRay Ray) const
{
	float MinDistance = MaxFloat;
	RRaycastTest ClosestHit{.Hit = false, .Distance = -1};

	const auto AabbMesh = GeometryCatalogue.find("aabb")->second;

	int PointC = 0;
	for (EPointLight* Light : this->PointLights)
	{
		// subtract Lightbulb model size from Position
		auto Position = Light->Position - vec3{0.1575, 0, 0.1575};
		auto AabbModel = translate(Mat4Identity, Position);
		AabbModel = scale(AabbModel, vec3{0.3f, 0.6f, 0.3f});

		auto Test = TestRayAgainstMesh(Ray, AabbMesh, AabbModel, RayCast_TestBothSidesOfTriangle);
		if (Test.Hit && Test.Distance < MinDistance)
		{
			ClosestHit = {true, Test.Distance, nullptr, PointC, "point"};
			MinDistance = Test.Distance;
		}
		PointC++;
	}

	int SpotC = 0;
	for (ESpotLight* Light : this->SpotLights)
	{
		// subtract Lightbulb model size from Position
		auto Position = Light->Position - vec3{0.1575, 0, 0.1575};
		auto AabbModel = translate(Mat4Identity, Position);
		AabbModel = scale(AabbModel, vec3{0.3f, 0.6f, 0.3f});

		const auto Test = TestRayAgainstMesh(Ray, AabbMesh, AabbModel, RayCast_TestBothSidesOfTriangle);
		if (Test.Hit && Test.Distance < MinDistance)
		{
			ClosestHit = {.Hit = true, .Distance = Test.Distance, .Entity = nullptr, .ObjHitIndex = SpotC, .ObjHitType = "spot"};
			MinDistance = Test.Distance;
		}
		SpotC++;
	}

	return ClosestHit;
}

CellUpdate RWorld::UpdateEntityWorldChunk(EEntity* Entity)
{
	string Message;

	// Computes which cells the entity is occupying based on its axis aligned bounding box
	auto [bb_min, bb_max] = Entity->BoundingBox.Bounds();
	auto [i0, j0, k0] = WorldCoordsToCells(bb_min);
	auto [i1, j1, k1] = WorldCoordsToCells(bb_max);

	// Out of bounds catch
	if (i0 == -1 || i1 == -1)
	{
		Message = "Entity '" + Entity->Name + "' is located out of current world bounds.";
		return CellUpdate{CellUpdate_OUT_OF_BOUNDS, Message};
	}

	// Unexpected output
	if (!(i1 >= i0 && j1 >= j0 && k1 >= k0))
	{
		Message = "Entity '" + Entity->Name + "' yielded invalid (inverted) world cell coordinates.";
		return CellUpdate{CellUpdate_UNEXPECTED, Message};
	}


	bool BChangedWc =
		Entity->WorldChunksCount == 0 ||
		Entity->WorldChunks[0]->i != i0 ||
		Entity->WorldChunks[0]->j != j0 ||
		Entity->WorldChunks[0]->k != k0 ||
		Entity->WorldChunks[Entity->WorldChunksCount - 1]->i != i1 ||
		Entity->WorldChunks[Entity->WorldChunksCount - 1]->j != j1 ||
		Entity->WorldChunks[Entity->WorldChunksCount - 1]->k != k1;

	if (!BChangedWc)
	{
		return CellUpdate{CellUpdate_OK, "", false};
	}

	const int NewCellsCount = (i1 - i0 + 1) * (j1 - j0 + 1) * (k1 - k0 + 1);

	// Entity too large catch
	if (NewCellsCount > MaxEntityWorldChunks)
	{
		Message = "Entity '" + Entity->Name + "' is too large and it occupies more than " +
		"the limit of " + std::to_string(MaxEntityWorldChunks) + " world cells at a time.";

		return CellUpdate{CellUpdate_ENTITY_TOO_BIG, Message};
	}

	// Remove entity from all world cells (inneficient due to defrag)
	for (int I = 0; I < Entity->WorldChunksCount; I++)
	{
		Entity->WorldChunks[I]->RemoveEntity(Entity);
	}
	Entity->WorldChunksCount = 0;

	auto OriginChunk = WorldCoordsToCells(Entity->Position);

	// Add entity to all world cells
	for (int I = i0; I <= i1; I++)
	{
		for (int J = j0; J <= j1; J++)
		{
			for (int K = k0; K <= k1; K++)
			{
				auto ChunkIt = this->ChunksMap.find({I, J, K});
				if (ChunkIt == ChunksMap.end())
					return CellUpdate{CellUpdate_OUT_OF_BOUNDS, "Coordinates not found in chunks_map.", true};

				auto* Chunk = ChunkIt->second;
				Entity->WorldChunks.push_back(Chunk);
			}
		}
	}

	return CellUpdate{CellUpdate_OK, "", true};
}

auto RWorld::GetFrameData() -> RavenousEngine::RFrameData&
{
	return RavenousEngine::REngineRuntimeState::Get()->Frame;
}

bool RWorld::IsEntitySlotValid(const REntitySlot& Slot) const
{
	return EntityStorage.SlotContainsEntity(&Slot);
}

// TODO: Move these elsewhere
void SetEntityDefaultAssets(EEntity* Entity)
{
	// Trusting type defaults
	REntityAttributes Attrs;

	auto [Textures, TextureCount, Mesh, CollisionMesh, Shader] = FindEntityAssetsInCatalogue(Attrs.Mesh, Attrs.CollisionMesh, Attrs.Shader, Attrs.Texture);
	Entity->Name = Attrs.Name;
	Entity->Shader = Shader;
	Entity->Mesh =  Mesh;
	Entity->Scale = Attrs.Scale;
	Entity->CollisionMesh =  CollisionMesh;
	Entity->Collider = * CollisionMesh;
	Entity->TextureDiffuse = Textures[0];
}

void SetEntityAssets(EEntity* Entity, REntityAttributes Attrs)
{
	auto [Textures, TextureCount, Mesh, CollisionMesh, Shader] = FindEntityAssetsInCatalogue(Attrs.Mesh, Attrs.CollisionMesh, Attrs.Shader, Attrs.Texture);
	Entity->Name = Attrs.Name;
	Entity->Shader = Shader;
	Entity->Mesh =  Mesh;
	Entity->Scale = Attrs.Scale;
	Entity->CollisionMesh =  CollisionMesh;
	Entity->Collider = * CollisionMesh;
	Entity->TextureDiffuse = Textures[0];
}