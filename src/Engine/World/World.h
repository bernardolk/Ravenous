#pragma once

#include "WorldChunk.h"
#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "engine/entities/Entity.h"
#include "engine/utils/utils.h"

namespace RavenousEngine
{
	struct RFrameData;
}
struct EEntity;
struct RWorldChunkPosition WorldCoordsToCells(float x, float y, float z);
vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);

enum CellUpdateStatus
{
	CellUpdate_OK,
	CellUpdate_ENTITY_TOO_BIG,
	CellUpdate_CELL_FULL,
	CellUpdate_OUT_OF_BOUNDS,
	CellUpdate_UNEXPECTED
};

struct CellUpdate
{
	CellUpdateStatus status = CellUpdate_UNEXPECTED;
	std::string message;
	bool entity_changed_cell = false;
};


struct RWorld
{
	DeclSingleton(RWorld)
	
	// static constexpr u8 world_chunk_matrix_order = 10;
	static constexpr u32 world_size_in_chunks = WorldChunkNumX * WorldChunkNumY * WorldChunkNumZ;

	string scene_name;

	// TODO: We can't use world chunk "matrix" position as its ijk position! This is insane! What if we want to unload part A of the world and load part B,
	//		what are the index going to say? Nothing.
	//		in the future such vector will be replaced with a memory arena
	Array<RWorldChunk, world_size_in_chunks> chunks;
	map<RWorldChunkPosition, RWorldChunk*> chunks_map;
	vector<RWorldChunk*> active_chunks;

	float global_shininess = 17;
	float ambient_intensity = 0;
	vec3 ambient_light = vec3(1);

	// temp
	vector<PointLight*> point_lights;
	vector<SpotLight*> spot_lights;
	vector<DirectionalLight*> directional_lights;
	// end temp
		
	void Update();

	template<typename T_Entity>
	T_Entity* SpawnEntity();

	template<typename T_Entity>
	T_Entity* RWorld::SpawnEntityAtPosition(vec3 position);

	Iterator<RWorldChunk> GetChunkIterator();
	static WorldEntityIterator GetEntityIterator();	
	
	RRaycastTest Raycast(RRay ray, NRayCastType test_type, const EEntity* skip = nullptr, float max_distance = MaxFloat) const;
	RRaycastTest Raycast(RRay ray, const EEntity* skip = nullptr, float max_distance = MaxFloat) const;
	RRaycastTest LinearRaycastArray(RRay first_ray, int qty, float spacing) const;
	RRaycastTest RaycastLights(RRay ray) const;

	CellUpdate UpdateEntityWorldChunk(EEntity* entity);

	RavenousEngine::RFrameData& GetFrameData();
	
private:
	void UpdateTraits();
	void UpdateTransforms();
};

struct WorldEntityIterator
{
	u8 total_active_chunks = 0;
	u8 current_chunk_index = 0;

	RWorld* world;
	RWorldChunkEntityIterator chunk_iterator;
	
	WorldEntityIterator();
	
	EEntity* operator()();
};


//TODO: Move these somewhere else
void SetEntityDefaultAssets(EEntity* entity);
void SetEntityAssets(EEntity* entity, struct REntityAttributes attrs);


template<typename T_Entity>
T_Entity* RWorld::SpawnEntity()
{
	auto* first_chunk_available = chunks_map.begin()->second;
	if (!first_chunk_available) return nullptr;

	return first_chunk_available->AddEntity<T_Entity>();
}

template<typename T_Entity>
T_Entity* RWorld::SpawnEntityAtPosition(vec3 position)
{
	auto* first_chunk_available = chunks_map.begin()->second;
	if (!first_chunk_available) return nullptr;

	auto* entity = first_chunk_available->RequestEntityStorage<T_Entity>();
	if (entity)
	{
		entity->position = position;
		UpdateEntityWorldChunk(entity);
	}
			
	return entity;
}
