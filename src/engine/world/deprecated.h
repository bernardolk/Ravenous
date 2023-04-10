/* World.h */

#pragma once

#include "world_chunk.h"
#include "engine/collision/primitives/bounding_box.h"

// auto WorldCoordsToCells(float x, float y, float z);
// vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);
//
// // how many cells we have preallocated for the world
// constexpr static int WCellsNumX = 10;
// constexpr static int WCellsNumY = 10;
// constexpr static int WCellsNumZ = 10;
//
// // how many cells are before and after the origin in each axis
// constexpr static int WCellsOffsetX = WCellsNumX / 2;
// constexpr static int WCellsOffsetY = WCellsNumY / 2;
// constexpr static int WCellsOffsetZ = WCellsNumZ / 2;
//
// // how many meters the cell occupies in the world
// constexpr static float WCellLenMeters = 50.0f;
// // how many entities can coexists in a cell
// constexpr static int WorldCellCapacity = 150;
//
// const static vec3 WUpperBoundsMeters = {
// WCellsOffsetX * WCellLenMeters,
// WCellsOffsetY * WCellLenMeters,
// WCellsOffsetZ * WCellLenMeters
// };
//
// const static vec3 WLowerBoundsMeters = {
// -1.0 * WCellsOffsetX * WCellLenMeters,
// -1.0 * WCellsOffsetX * WCellLenMeters,
// -1.0 * WCellsOffsetX * WCellLenMeters
// };
//
// enum CellUpdateStatus
// {
// 	CellUpdate_OK,
// 	CellUpdate_ENTITY_TOO_BIG,
// 	CellUpdate_CELL_FULL,
// 	CellUpdate_OUT_OF_BOUNDS,
// 	CellUpdate_UNEXPECTED
// };
//
// struct CellUpdate
// {
// 	CellUpdateStatus status{};
// 	std::string message{};
// 	bool entity_changed_cell = false;
// };

struct Entity;
struct BoundingBox;
struct Ray;
struct RaycastTest;
enum RayCastType;
struct PointLight;
struct SpotLight;
struct DirectionalLight;
struct Player;
struct EntityManager;

// -----------
// WORLD CELL
// -----------
// World cells are the way we spatially partition the game world. These are AABB's that contain a list of entities that touch them.
// Entities also have a list of the world cells they are currently living in.
// The reason is that in this game, things are more static than dynamic, so performing work on the lists on position changes is not
// that expensive (because it doesn't happen as often) and is useful to have a entity->world_cells link like this for various checks.
// WORLD CELLS ORIGINS LIE TO THE NEGATIVE AND GROW TOWARDS THE POSITIVE DIRECTION OF THEIR AXIS.
// So index 0 is at -50 (e.g.) and the cell extends until -40 (when we have length = 10).

struct WorldCell
{
	Entity* entities[WorldCellCapacity]{};
	unsigned int count = 0;

	// logical coords
	int i = -1, j = -1, k = -1;
	vec3 ijk{i, j, k};

	// world coords / bounding box
	BoundingBox bounding_box{};

	void Init(int ii, int ji, int ki);
	void Remove(Entity* entity);
	CellUpdate Add(Entity* entity);
	void Defrag();
	std::string CoordsStr() const;
	vec3 Coords() const;
	vec3 CoordsMeters() const;
	std::string CoordsMetersStr();
};

// -------------------
// COORDINATE METHODS
// -------------------

// inline vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k)
// {
// 	const float world_x = (static_cast<float>(i) - WCellsOffsetX) * WCellLenMeters;
// 	const float world_y = (static_cast<float>(j) - WCellsOffsetY) * WCellLenMeters;
// 	const float world_z = (static_cast<float>(k) - WCellsOffsetZ) * WCellLenMeters;
//
// 	return vec3{world_x, world_y, world_z};
// }
//
//
// inline auto WorldCoordsToCells(float x, float y, float z)
// {
// 	struct
// 	{
// 		int i = -1, j = -1, k = -1;
// 	} world_cell_coords;
//
// 	// if out of bounds return -1
// 	if (x < WLowerBoundsMeters.x || x > WUpperBoundsMeters.x ||
// 		y < WLowerBoundsMeters.y || y > WUpperBoundsMeters.y ||
// 		z < WLowerBoundsMeters.z || z > WUpperBoundsMeters.z)
// 	{
// 		return world_cell_coords;
// 	}
//
// 	// int division to truncate float result to correct cell position
// 	world_cell_coords.i = (x + WCellsOffsetX * WCellLenMeters) / WCellLenMeters;
// 	world_cell_coords.j = (y + WCellsOffsetY * WCellLenMeters) / WCellLenMeters;
// 	world_cell_coords.k = (z + WCellsOffsetZ * WCellLenMeters) / WCellLenMeters;
//
// 	return world_cell_coords;
// }
//
// inline auto WorldCoordsToCells(vec3 position)
// {
// 	return WorldCoordsToCells(position.x, position.y, position.z);
// }


// ----------------
// > WORLD
// ----------------
struct World
{
	// Entities lists
	std::vector<Entity*> entities;
	std::vector<Entity*> interactables;
	std::vector<Entity*> checkpoints;
	std::vector<PointLight*> point_lights;
	std::vector<SpotLight*> spot_lights;
	std::vector<DirectionalLight*> directional_lights;

	Player* player = nullptr;

	float global_shininess = 17;
	float ambient_intensity = 0;
	vec3 ambient_light = vec3(1);

	WorldCell cells[WorldChunkNumX][WorldChunkNumY][WorldChunkNumZ];
	WorldCell* cells_in_use[WorldChunkNumX * WorldChunkNumY * WorldChunkNumZ]{};
	int cells_in_use_count = 0;

public:
	static World* Get()
	{
		static World instance;
		return &instance;
	}
	void Init();
	void UpdateCellsInUseList();
	void UpdateEntities();
	void Clear(const EntityManager* manager);

	RaycastTest Raycast(Ray ray, RayCastType test_type, const Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest Raycast(Ray ray, const Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest LinearRaycastArray(Ray first_ray, int qty, float spacing) const;
	RaycastTest RaycastLights(Ray ray) const;
	CellUpdate UpdateEntityWorldCells(Entity* entity);

private:
	World();
	World(const World& other) = delete;
};

/* World.cpp */

#include <engine/core/core.h>
#include <string>
#include "engine/utils/utils.h"
#include <engine/collision/primitives/bounding_box.h>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include "engine/geometry/mesh.h"
#include <glm/gtx/normal.hpp>
#include "engine/utils/colors.h"
#include <engine/render/im_render.h>
#include <engine/collision/primitives/ray.h>
#include <engine/collision/raycast.h>
#include "engine/entities/entity.h"
#include "game/entities/player.h"
#include "engine/entities/lights.h"
#include "engine/entities/allocator/entity_pool.h"
#include "engine/entities/manager/entity_manager.h"
#include <engine/world/world.h>

#include "engine/core/platform.h"


void WorldCell::Init(int ii, int ji, int ki)
{
	this->count = 0;

	// set logical coordinates
	this->i = ii;
	this->j = ji;
	this->k = ki;
	this->ijk = vec3{ii, ji, ki};

	// set physical world coordinates to bounding box
	const vec3 origin = GetWorldCoordinatesFromWorldCellCoordinates(ii, ji, ki);
	this->bounding_box.minx = origin.x;
	this->bounding_box.miny = origin.y;
	this->bounding_box.minz = origin.z;
	this->bounding_box.maxx = origin.x + WorldChunkLengthMeters;
	this->bounding_box.maxy = origin.y + WorldChunkLengthMeters;
	this->bounding_box.maxz = origin.z + WorldChunkLengthMeters;

	// initialize entities list
	for (int i = 0; i < WorldCellCapacity; i++)
		this->entities[i] = nullptr;
}


void WorldCell::Remove(Entity* entity)
{
	for (int i = 0; i < WorldCellCapacity; i++)
		if (this->entities[i] == entity)
		{
			this->entities[i] = nullptr;
			this->Defrag();
			return;
		}
}


CellUpdate WorldCell::Add(Entity* entity)
{
	if (count == WorldCellCapacity)
	{
		const auto message = "World cell '" + this->CoordsStr() + "' is full.";
		return CellUpdate{CellUpdate_CELL_FULL, message};
	}

	for (int i = 0; i < WorldCellCapacity; i++)
		if (this->entities[i] == nullptr)
		{
			this->entities[i] = entity;
			this->count++;
			return CellUpdate{CellUpdate_OK};
		}

	return CellUpdate{CellUpdate_UNEXPECTED, "world cell add method returned weirdly."};
}


//@TODO: Refactor this whole thing to use mempool
void WorldCell::Defrag()
{
	if (this->count == 0)
		return;

	// initialize holes array
	unsigned int hole_count = 0;
	int holes[WorldCellCapacity];
	for (int i = 0; i < WorldCellCapacity; i++)
		holes[i] = -1;

	// find holes and store in array
	// also count how many items there are
	int new_count = 0;
	for (int i = 0; i < this->count; i++)
	{
		// we dont want to count the last empty spot as a hole
		if (this->entities[i] == nullptr)
		{
			if (i + 1 != this->count)
				holes[hole_count++] = i;
		}
		else
			new_count++;
	}

	// loop through list from top to bottom and fill
	// holes as it finds candidates to swap      
	int idx = this->count - 1;
	int hole_idx = 0;
	while (true)
	{
		int hole = holes[hole_idx];
		if (hole == -1 || idx == 0)
			break;

		auto item = this->entities[idx];
		if (item != nullptr)
		{
			this->entities[hole] = item;
			this->entities[idx] = nullptr;
			hole_idx++;
		}
		idx--;
	}
	this->count = new_count;
}


std::string WorldCell::CoordsStr() const
{
	return "Cell [" + std::to_string(this->i)
	+ "," + std::to_string(this->j) + "," + std::to_string(this->k)
	+ "] (" + std::to_string(this->count) + ")";
}


vec3 WorldCell::Coords() const
{
	return vec3{this->i, this->j, this->k};
}


vec3 WorldCell::CoordsMeters() const
{
	return GetWorldCoordinatesFromWorldCellCoordinates(this->i, this->j, this->k);
}


std::string WorldCell::CoordsMetersStr()
{
	vec3 mcoords = this->CoordsMeters();
	return "[x: " + FormatFloatTostr(mcoords[0], 1)
	+ ", y: " + FormatFloatTostr(mcoords[1], 1) + ", z: " + FormatFloatTostr(mcoords[2], 1)
	+ "]";
}


World::World()
{
	this->Init();
}


void World::Init()
{
	for (int i = 0; i < WorldChunkNumX; i++)
		for (int j = 0; j < WorldChunkNumY; j++)
			for (int k = 0; k < WorldChunkNumZ; k++)
				this->cells[i][j][k].Init(i, j, k);
}


void World::UpdateCellsInUseList()
{
	this->cells_in_use_count = 0;
	for (int i = 0; i < WorldChunkNumX; i++)
		for (int j = 0; j < WorldChunkNumY; j++)
			for (int k = 0; k < WorldChunkNumZ; k++)
			{
				const auto cell = &this->cells[i][j][k];
				if (cell->count != 0)
					cells_in_use[this->cells_in_use_count++] = cell;
			}
}


CellUpdate World::UpdateEntityWorldCells(Entity* entity)
{
	std::string message;

	// Computes which cells the entity is occupying based on its axis aligned bounding box
	auto [bb_min, bb_max] = entity->bounding_box.Bounds();
	auto [i0, j0, k0] = WorldCoordsToCells(bb_min);
	auto [i1, j1, k1] = WorldCoordsToCells(bb_max);

	// Out of bounds catch
	if (i0 == -1 || i1 == -1)
	{
		message = "Entity '" + entity->name + "' is located out of current world bounds.";
		return CellUpdate{CellUpdate_OUT_OF_BOUNDS, message};
	}

	// Unexpected output
	if (!(i1 >= i0 && j1 >= j0 && k1 >= k0))
	{
		message = "Entity '" + entity->name + "' yielded invalid (inverted) world cell coordinates.";
		return CellUpdate{CellUpdate_UNEXPECTED, message};
	}


	bool b_changed_wc =
	entity->world_cells_count == 0 ||
	entity->world_cells[0]->i != i0 ||
	entity->world_cells[0]->j != j0 ||
	entity->world_cells[0]->k != k0 ||
	entity->world_cells[entity->world_cells_count - 1]->i != i1 ||
	entity->world_cells[entity->world_cells_count - 1]->j != j1 ||
	entity->world_cells[entity->world_cells_count - 1]->k != k1;

	if (!b_changed_wc)
	{
		return CellUpdate{CellUpdate_OK, "", false};
	}

	const int new_cells_count = (i1 - i0 + 1) * (j1 - j0 + 1) * (k1 - k0 + 1);

	// Entity too large catch
	if (new_cells_count > MaxEntityWorldChunks)
	{
		message = "Entity '" + entity->name + "' is too large and it occupies more than " +
		"the limit of " + std::to_string(MaxEntityWorldChunks) + " world cells at a time.";

		return CellUpdate{CellUpdate_ENTITY_TOO_BIG, message};
	}

	// Remove entity from all world cells (inneficient due to defrag)
	for (int i = 0; i < entity->world_cells_count; i++)
	{
		entity->world_cells[i]->Remove(entity);
	}
	entity->world_cells_count = 0;


	// Add entity to all world cells
	for (int i = i0; i <= i1; i++)
	{
		for (int j = j0; j <= j1; j++)
		{
			for (int k = k0; k <= k1; k++)
			{
				auto* cell = &this->cells[i][j][k];
				auto cell_update = cell->Add(entity);
				if (cell_update.status != CellUpdate_OK)
				{
					// TODO: not sure returning in the middle of this process is cool...
					return cell_update;
				}
				entity->world_cells[entity->world_cells_count++] = cell;
			}
		}
	}

	return CellUpdate{CellUpdate_OK, "", true};
}


RaycastTest World::Raycast(const Ray ray, const RayCastType test_type, const Entity* skip, const float max_distance) const
{
	//@TODO: This should first test ray against world cells, then get the list of entities from these world cells to test against 

DEPRECATED_BEGIN
	/*
	float min_distance = MaxFloat;
	RaycastTest closest_hit{false, -1};

	for (const auto entity : this->entities)
	{
		if (test_type == RayCast_TestOnlyVisibleEntities && entity->flags & EntityFlags_InvisibleEntity)
			continue;
		if (skip != nullptr && entity->id == skip->id)
			continue;

		const auto test = CL_TestAgainstRay(ray, entity, test_type, max_distance);
		if (test.hit && test.distance < min_distance && test.distance < max_distance)
		{
			closest_hit = test;
			closest_hit.entity = entity;
			min_distance = test.distance;
		}
	}

	return closest_hit;
	*/
DEPRECATED_END

	return {};
}


RaycastTest World::Raycast(const Ray ray, const Entity* skip, const float max_distance) const
{
	return this->Raycast(ray, RayCast_TestOnlyFromOutsideIn, skip, max_distance);
}


RaycastTest World::LinearRaycastArray(const Ray first_ray, int qty, float spacing) const
{
	/* 
	   Casts multiple ray towards the first_ray direction, with dir pointing upwards,
	   qty says how many rays to shoot and spacing, well, the spacing between each ray.
	*/
	DEPRECATED_BEGIN
		/*
	
	Ray ray = first_ray;
	float highest_y = MinFloat;
	float shortest_z = MaxFloat;
	RaycastTest best_hit_results;

	ForLess(qty)
	{
		auto test = this->Raycast(ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr, player->grab_reach);
		if (test.hit)
		{
			if (test.distance < shortest_z || (AreEqualFloats(test.distance, shortest_z) && highest_y < ray.origin.y))
			{
				highest_y = ray.origin.y;
				shortest_z = test.distance;
				best_hit_results = test;
			}
		}

		ImDraw::AddLine(IM_ITERHASH(i), ray.origin, ray.origin + ray.direction * player->grab_reach, 1.2f, false, COLOR_GREEN_1);

		ray = Ray{ray.origin + UnitY * spacing, ray.direction};
	}

	if (best_hit_results.hit)
	{
		vec3 hitpoint = CL_GetPointFromDetection(best_hit_results.ray, best_hit_results);
		ImDraw::AddPoint(IMHASH, hitpoint, 2.0, true, COLOR_RED_1);
	}


	return best_hit_results;

	*/
DEPRECATED_END

	return {};
}

RaycastTest World::RaycastLights(const Ray ray) const
{
	float min_distance = MaxFloat;
	RaycastTest closest_hit{.hit = false, .distance = -1};

	const auto aabb_mesh = GeometryCatalogue.find("aabb")->second;

	int point_c = 0;
	for (auto& light : this->point_lights)
	{
		// subtract lightbulb model size from position
		auto position = light->position - vec3{0.1575, 0, 0.1575};
		auto aabb_model = translate(Mat4Identity, position);
		aabb_model = scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

		auto test = CL_TestAgainstRay(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
		if (test.hit && test.distance < min_distance)
		{
			closest_hit = {true, test.distance, nullptr, point_c, "point"};
			min_distance = test.distance;
		}
		point_c++;
	}

	int spot_c = 0;
	for (auto& light : this->spot_lights)
	{
		// subtract lightbulb model size from position
		auto position = light->position - vec3{0.1575, 0, 0.1575};
		auto aabb_model = translate(Mat4Identity, position);
		aabb_model = scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

		const auto test = CL_TestAgainstRay(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
		if (test.hit && test.distance < min_distance)
		{
			closest_hit = {
			.hit = true, .distance = test.distance, .entity = nullptr, .obj_hit_index = spot_c, .obj_hit_type = "spot"
			};
			min_distance = test.distance;
		}
		spot_c++;
	}

	return closest_hit;
}


void World::Clear(const EntityManager* manager)
{
	/*
	   This is the ~official~ world unloading/clearing procedure.
	   This sets all pool slots to free and removes all entities allocated with new.
	*/

	// drops all entities
	for (const auto& entity : this->entities)
		manager->pool.FreeSlot(entity);

	this->entities.clear();

	// drops all lights
	for (const auto& light : this->point_lights)
		delete light;

	this->point_lights.clear();

	for (const auto& light : this->spot_lights)
		delete light;

	this->spot_lights.clear();

	for (const auto& light : this->directional_lights)
		delete light;

	this->directional_lights.clear();

	// drops all interactable entities
	for (const auto& interactable : this->interactables)
		delete interactable;

	this->interactables.clear();

	// drops all checkpoint entities
	for (const auto& checkpoint : this->checkpoints)
		delete checkpoint;

	this->checkpoints.clear();
}

void World::UpdateEntities()
{
	for (const auto& entity : this->entities)
	{
		entity->Update();
		UpdateEntityWorldCells(entity);
	}
}