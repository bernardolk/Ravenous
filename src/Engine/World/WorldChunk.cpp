#include "WorldChunk.h"

#include "engine/utils/utils.h"


WorldChunkEntityIterator WorldChunk::GetIterator()
{
	return WorldChunkEntityIterator(this);
}

void WorldChunk::RemoveEntity(EEntity* entity_to_delete)
{
	// TODO: We are not dealing with actually removing entities yet, we only mark them so they can be skipped/overwritten.
	entity_to_delete->deleted = true;
}


bool WorldChunk::AddVisitor(EEntity* entity)
{
	visitors.push_back(entity);
	entity->visitor_state = VisitorState{true, vec3(i, j, k), this};
	return true;
}

bool WorldChunk::RemoveVisitor(EEntity* entity)
{
	int i = 0;
	for (auto* visitor: visitors)
	{
		// TODO: #PtrToEntity Change to handle futurely
		if (visitor == entity)
		{
			visitors.erase(visitors.begin() + i);
			entity->visitor_state.Reset();
		}
		
		i++;
	}
	
	return false;
}

vec3 WorldChunk::GetPositionMetric()
{
	return GetWorldCoordinatesFromWorldCellCoordinates(i, j , k);
}

string WorldChunk::GetChunkPositionString()
{
	return "Cell [" + to_string(i) + "," + to_string(j) + "," + to_string(k) + "]";
}

string WorldChunk::GetChunkPositionMetricString()
{
	vec3 mcoords = GetPositionMetric();
	return "[x: " + FormatFloatTostr(mcoords[0], 1)
	+ ", y: " + FormatFloatTostr(mcoords[1], 1) + ", z: " + FormatFloatTostr(mcoords[2], 1)
	+ "]";
}

void WorldChunk::InvokeTraitUpdateOnAllTypes(TraitID trait_id)
{
	auto* etm = EntityTraitsManager::Get();
	//auto* types = etm->GetTypesWithTrait(trait_id);
	
	for (auto& block_metadata : chunk_storage.storage_metadata_array)
	{
		if (block_metadata.entity_traits.Contains(trait_id))
		{
			auto* trait_update_func = etm->GetUpdateFunc(block_metadata.type_id, trait_id);
			for (int i = 0; i < block_metadata.entity_count; i++)
			{
				auto* entity = reinterpret_cast<EEntity*>(block_metadata.data_start + block_metadata.type_size * i);
				trait_update_func(entity);
			}
		}
	}
}

vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k)
{
	const float world_x = (static_cast<float>(i) - WorldChunkOffsetX) * WorldChunkLengthMeters;
	const float world_y = (static_cast<float>(j) - WorldChunkOffsetY) * WorldChunkLengthMeters;
	const float world_z = (static_cast<float>(k) - WorldChunkOffsetZ) * WorldChunkLengthMeters;

	return vec3{world_x, world_y, world_z};
}

WorldChunkPosition WorldCoordsToCells(float x, float y, float z)
{
	WorldChunkPosition world_cell_coords;

	// if out of bounds return -1
	if (x < WLowerBoundsMeters.x || x > WUpperBoundsMeters.x ||
		y < WLowerBoundsMeters.y || y > WUpperBoundsMeters.y ||
		z < WLowerBoundsMeters.z || z > WUpperBoundsMeters.z)
	{
		return world_cell_coords;
	}

	// int division to truncate float result to correct cell position
	world_cell_coords.i = (x + WorldChunkOffsetX * WorldChunkLengthMeters) / WorldChunkLengthMeters;
	world_cell_coords.j = (y + WorldChunkOffsetY * WorldChunkLengthMeters) / WorldChunkLengthMeters;
	world_cell_coords.k = (z + WorldChunkOffsetZ * WorldChunkLengthMeters) / WorldChunkLengthMeters;

	return world_cell_coords;
}

WorldChunkPosition WorldCoordsToCells(vec3 position)
{
	return WorldCoordsToCells(position.x, position.y, position.z);
}