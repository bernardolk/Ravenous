
#include "engine/utils/utils.h"
#include "Engine/Entities/Entity.h"
#include "WorldChunk.h"

RWorldChunkEntityIterator RWorldChunk::GetIterator()
{
	return RWorldChunkEntityIterator(this);
}

void RWorldChunk::RemoveEntity(EEntity* EntityToDelete)
{
	// TODO: We are not dealing with actually removing entities yet, we only mark them so they can be skipped/overwritten.
	EntityToDelete->Deleted = true;
}


bool RWorldChunk::AddVisitor(EEntity* Entity)
{
	Visitors.push_back(Entity);
	Entity->VisitorState = VisitorState{true, vec3(i, j, k), this};
	return true;
}

bool RWorldChunk::RemoveVisitor(EEntity* Entity)
{
	int Count = 0;
	for (auto* Visitor : Visitors)
	{
		// TODO: #PtrToEntity Change to handle futurely
		if (Visitor == Entity)
		{
			Visitors.erase(Visitors.begin() + Count);
			Entity->VisitorState.Reset();
		}

		Count++;
	}

	return false;
}

vec3 RWorldChunk::GetPositionMetric()
{
	return GetWorldCoordinatesFromWorldCellCoordinates(i, j, k);
}

string RWorldChunk::GetChunkPositionString()
{
	return "Cell [" + to_string(i) + "," + to_string(j) + "," + to_string(k) + "]";
}

string RWorldChunk::GetChunkPositionMetricString()
{
	vec3 MetricCoords = GetPositionMetric();
	return "[x: " + FormatFloatTostr(MetricCoords[0], 1)
	+ ", y: " + FormatFloatTostr(MetricCoords[1], 1) + ", z: " + FormatFloatTostr(MetricCoords[2], 1)
	+ "]";
}

void RWorldChunk::InvokeTraitUpdateOnAllTypes(RTraitID TraitId)
{
	auto* TraitsManager = EntityTraitsManager::Get();
	for (auto& BlockMetadata : ChunkStorage.StorageMetadataArray)
	{
		if (BlockMetadata.EntityTraits.Contains(TraitId))
		{
			auto* TraitUpdateFunc = TraitsManager->GetUpdateFunc(BlockMetadata.TypeID, TraitId);
			for (int i = 0; i < BlockMetadata.EntityCount; i++)
			{
				auto* Entity = reinterpret_cast<EEntity*>(BlockMetadata.DataStart + BlockMetadata.TypeSize * i);
				TraitUpdateFunc(Entity);
			}
		}
	}
}

vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k)
{
	const float WorldX = (static_cast<float>(i) - WorldChunkOffsetX) * WorldChunkLengthMeters;
	const float WorldY = (static_cast<float>(j) - WorldChunkOffsetY) * WorldChunkLengthMeters;
	const float WorldZ = (static_cast<float>(k) - WorldChunkOffsetZ) * WorldChunkLengthMeters;

	return vec3{WorldX, WorldY, WorldZ};
}

RWorldChunkPosition WorldCoordsToCells(float X, float Y, float Z)
{
	RWorldChunkPosition WorldCellCoords;

	// if out of bounds return -1
	if (X < WLowerBoundsMeters.x || X > WUpperBoundsMeters.x ||
		Y < WLowerBoundsMeters.y || Y > WUpperBoundsMeters.y ||
		Z < WLowerBoundsMeters.z || Z > WUpperBoundsMeters.z)
	{
		return WorldCellCoords;
	}

	// int division to truncate float result to correct cell position
	WorldCellCoords.i = (X + WorldChunkOffsetX * WorldChunkLengthMeters) / WorldChunkLengthMeters;
	WorldCellCoords.j = (Y + WorldChunkOffsetY * WorldChunkLengthMeters) / WorldChunkLengthMeters;
	WorldCellCoords.k = (Z + WorldChunkOffsetZ * WorldChunkLengthMeters) / WorldChunkLengthMeters;

	return WorldCellCoords;
}

RWorldChunkPosition WorldCoordsToCells(vec3 Position)
{
	return WorldCoordsToCells(Position.x, Position.y, Position.z);
}
