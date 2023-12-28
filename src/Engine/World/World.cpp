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

void RWorld::UpdateTransforms()
{
	auto EntityIter = GetEntityIterator();
	while (auto* Entity = EntityIter())
	{
		Entity->Update();
	}
}

void RWorld::UpdateTraits()
{
	auto* TraitsManager = EntityTraitsManager::Get();
	for (RTraitID TraitId : TraitsManager->EntityTraits)
	{
		for (auto* Chunk : ActiveChunks)
		{
			Chunk->InvokeTraitUpdateOnAllTypes(TraitId);
		}
	}
}

TIterator<RWorldChunk> RWorld::GetChunkIterator()
{
	return Chunks.GetIterator();
}

WorldEntityIterator RWorld::GetEntityIterator()
{
	return WorldEntityIterator();
}

WorldEntityIterator::WorldEntityIterator() :
	World(RWorld::Get()), ChunkIterator(World->ActiveChunks[0]->GetIterator())
{
	TotalActiveChunks = World->ActiveChunks.size();
}

EEntity* WorldEntityIterator::operator()()
{
	if (EntityVectorIndex < World->EntityList.size()) {
		return World->EntityList[EntityVectorIndex++];
	}
	else {
		return nullptr;
	}
	
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

RRaycastTest RWorld::Raycast(const RRay Ray, const NRayCastType TestType, const EEntity* Skip, const float MaxDistance) const
{
	//@TODO: This should first Test Ray against world cells, then get the list of entities from these world cells to Test against 

	float MinDistance = MaxFloat;
	RRaycastTest ClosestHit{false, -1};

	auto EntityIterator = GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		if (TestType == RayCast_TestOnlyVisibleEntities && Entity->Flags & EntityFlags_InvisibleEntity)
			continue;

		if (Skip != nullptr && Entity->ID == Skip->ID)
			continue;

		const auto Test = ClTestAgainstRay(Ray, Entity, TestType, MaxDistance);
		if (Test.Hit && Test.Distance < MinDistance && Test.Distance < MaxDistance)
		{
			ClosestHit = Test;
			ClosestHit.Entity = Entity;
			MinDistance = Test.Distance;
		}
	}

	return ClosestHit;
}

RRaycastTest RWorld::Raycast(const RRay Ray, const EEntity* Skip, const float MaxDistance) const
{
	return this->Raycast(Ray, RayCast_TestOnlyFromOutsideIn, Skip, MaxDistance);
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

		RImDraw::AddLine(IM_ITERHASH(i), Ray.Origin, Ray.Origin + Ray.Direction * Player->GrabReach, 1.2f, false, COLOR_GREEN_1);

		Ray = RRay{Ray.Origin + UnitY * Spacing, Ray.Direction};
	}

	if (BestHitResults.Hit)
	{
		vec3 Hitpoint = ClGetPointFromDetection(BestHitResults.Ray, BestHitResults);
		RImDraw::AddPoint(IMHASH, Hitpoint, 2.0, true, COLOR_RED_1);
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

		auto Test = ClTestAgainstRay(Ray, AabbMesh, AabbModel, RayCast_TestBothSidesOfTriangle);
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

		const auto Test = ClTestAgainstRay(Ray, AabbMesh, AabbModel, RayCast_TestBothSidesOfTriangle);
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

				if (OriginChunk != vec3(I, J, K))
					Chunk->AddVisitor(Entity);

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

	for (int i = 0; i < TextureCount; i++) {
		Entity->Textures.push_back(Textures[i]);
	}
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

	for (int i = 0; i < TextureCount; i++) {
		Entity->Textures.push_back( Textures[i]);
	}
}
