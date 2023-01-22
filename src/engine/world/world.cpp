#include <engine/core/types.h>
#include <iostream> // utils
#include <string>
#include <engine/vertex.h> // utils
#include <algorithm> // utils
#include <utils.h>
#include <engine/collision/primitives/bounding_box.h>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <engine/mesh.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <colors.h>
#include <engine/render/renderer.h>
#include <engine/render/im_render.h>
#include <engine/collision/primitives/ray.h>
#include <engine/collision/raycast.h>
#include <engine/collision/collision_mesh.h>
#include "engine/entities/entity.h"
#include <player.h>
#include <engine/lights.h>
#include <engine/logging.h>
#include <engine/entity_pool.h>
#include <engine/entity_manager.h>
#include <engine/world/world.h>


void WorldCell::Init(int ii, int ji, int ki)
{
	this->count = 0;

	// set logical coordinates
	this->i = ii;
	this->j = ji;
	this->k = ki;

	// set physical world coordinates to bounding box
	const vec3 origin = get_world_coordinates_from_world_cell_coordinates(ii, ji, ki);
	this->bounding_box.minx = origin.x;
	this->bounding_box.miny = origin.y;
	this->bounding_box.minz = origin.z;
	this->bounding_box.maxx = origin.x + WCellLenMeters;
	this->bounding_box.maxy = origin.y + WCellLenMeters;
	this->bounding_box.maxz = origin.z + WCellLenMeters;

	// initialize entities list
	for(int i = 0; i < WorldCellCapacity; i++)
		this->entities[i] = nullptr;
}


void WorldCell::Remove(Entity* entity)
{
	for(int i = 0; i < WorldCellCapacity; i++)
		if(this->entities[i] == entity)
		{
			this->entities[i] = nullptr;
			this->Defrag();
			return;
		}
}


CellUpdate WorldCell::Add(Entity* entity)
{
	if(count == WorldCellCapacity)
	{
		const auto message = "World cell '" + this->CoordsStr() + "' is full.";
		return CellUpdate{CellUpdate_CELL_FULL, message};
	}

	for(int i = 0; i < WorldCellCapacity; i++)
		if(this->entities[i] == nullptr)
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
	if(this->count == 0)
		return;

	// initialize holes array
	unsigned int hole_count = 0;
	int holes[WorldCellCapacity];
	for(int i = 0; i < WorldCellCapacity; i++)
		holes[i] = -1;

	// find holes and store in array
	// also count how many items there are
	int new_count = 0;
	for(int i = 0; i < this->count; i++)
	{
		// we dont want to count the last empty spot as a hole
		if(this->entities[i] == nullptr)
		{
			if(i + 1 != this->count)
				holes[hole_count++] = i;
		}
		else
			new_count++;
	}

	// loop through list from top to bottom and fill
	// holes as it finds candidates to swap      
	int idx = this->count - 1;
	int hole_idx = 0;
	while(true)
	{
		int hole = holes[hole_idx];
		if(hole == -1 || idx == 0)
			break;

		auto item = this->entities[idx];
		if(item != nullptr)
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
	return get_world_coordinates_from_world_cell_coordinates(this->i, this->j, this->k);
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
	for(int i = 0; i < WCellsNumX; i++)
		for(int j = 0; j < WCellsNumY; j++)
			for(int k = 0; k < WCellsNumZ; k++)
				this->cells[i][j][k].Init(i, j, k);
}


void World::UpdateCellsInUseList()
{
	this->cells_in_use_count = 0;
	for(int i = 0; i < WCellsNumX; i++)
		for(int j = 0; j < WCellsNumY; j++)
			for(int k = 0; k < WCellsNumZ; k++)
			{
				const auto cell = &this->cells[i][j][k];
				if(cell->count != 0)
					cells_in_use[this->cells_in_use_count++] = cell;
			}
}


CellUpdate World::UpdateEntityWorldCells(Entity* entity)
{
	std::string message;

	// computes which cells the entity is occupying based on it's axis aligned bounding box
	auto [bb_min, bb_max] = entity->bounding_box.Bounds();
	auto [i0, j0, k0] = world_coords_to_cells(bb_min);
	auto [i1, j1, k1] = world_coords_to_cells(bb_max);

	// out of bounds catch
	if(i0 == -1 || i1 == -1)
	{
		message = "Entity '" + entity->name + "' is located out of current world bounds.";
		return CellUpdate{CellUpdate_OUT_OF_BOUNDS, message};
	}

	std::vector<WorldCell*> new_cells;
	for(int i = i0; i <= i1; i++)
		for(int j = j0; j <= j1; j++)
			for(int k = k0; k <= k1; k++)
				new_cells.push_back(&this->cells[i][j][k]);

	// entity too large catch
	if(new_cells.size() > EntityWolrdCellOccupationLimit)
	{
		message = "Entity '" + entity->name + "' is too large and it occupies more than " +
		"the limit of " + std::to_string(EntityWolrdCellOccupationLimit) + " world cells at a time.";

		return CellUpdate{CellUpdate_ENTITY_TOO_BIG, message};
	}

	// computes outdated world cells to remove the entity from
	std::vector<WorldCell*> cells_to_remove_from;
	for(int i = 0; i < entity->world_cells_count; i++)
	{
		auto entity_world_cell = entity->world_cells[i];
		bool found = false;
		for(int c = 0; c < new_cells.size(); c++)
		{
			if(new_cells[c] == entity_world_cell)
			{
				found = true;
				break;
			}
		}
		if(!found)
			cells_to_remove_from.push_back(entity_world_cell);
	}

	// computes the cells to add entity to
	std::vector<WorldCell*> cells_to_add_to;
	for(int i = 0; i < new_cells.size(); i++)
	{
		auto cell = new_cells[i];
		bool exists = false;
		for(int j = 0; j < entity->world_cells_count; j++)
		{
			auto entity_cell = entity->world_cells[j];
			if(cell == entity_cell)
			{
				exists = true;
				break;
			}
		}
		if(!exists)
			cells_to_add_to.push_back(cell);
	}

	// remove entity from old cells
	for(int i = 0; i < cells_to_remove_from.size(); i++)
	{
		auto cell = cells_to_remove_from[i];
		cell->Remove(entity);
	}

	// add entity to new cells
	// PS: if this fails, entity will have no cells. Easier to debug.
	for(int i = 0; i < cells_to_add_to.size(); i++)
	{
		auto cell = cells_to_add_to[i];
		auto cell_update = cell->Add(entity);
		if(cell_update.status != CellUpdate_OK)
		{
			return cell_update;
		}
	}

	// re-set cells to entity
	int new_cells_count = 0;
	for(int i = 0; i < new_cells.size(); i++)
	{
		auto cell = new_cells[i];
		entity->world_cells[new_cells_count] = cell;
		new_cells_count++;
	}
	entity->world_cells_count = new_cells_count;

	bool changed_cells = cells_to_add_to.size() > 0 || cells_to_remove_from.size() > 0;
	return CellUpdate{CellUpdate_OK, "", changed_cells};
}


RaycastTest World::Raycast(const Ray ray, const RayCastType test_type, const Entity* skip, const float max_distance) const
{
	//@TODO: This should first test ray against world cells, then get the list of entities from these world cells to test against 

	float min_distance = MaxFloat;
	RaycastTest closest_hit{false, -1};

	for(const auto entity : this->entities)
	{
		if(test_type == RayCast_TestOnlyVisibleEntities && entity->flags & EntityFlags_InvisibleEntity)
			continue;
		if(skip != nullptr && entity->id == skip->id)
			continue;

		const auto test = test_ray_against_entity(ray, entity, test_type, max_distance);
		if(test.hit && test.distance < min_distance && test.distance < max_distance)
		{
			closest_hit = test;
			closest_hit.entity = entity;
			min_distance = test.distance;
		}
	}

	return closest_hit;
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

	Ray ray = first_ray;
	float highest_y = MinFloat;
	float shortest_z = MaxFloat;
	RaycastTest best_hit_results;

	ForLess(qty)
	{
		auto test = this->Raycast(ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr, player->grab_reach);
		if(test.hit)
		{
			if(test.distance < shortest_z || (AreEqualFloats(test.distance, shortest_z) && highest_y < ray.origin.y))
			{
				highest_y = ray.origin.y;
				shortest_z = test.distance;
				best_hit_results = test;
			}
		}

		ImDraw::AddLine(IM_ITERHASH(i), ray.origin, ray.origin + ray.direction * player->grab_reach, 1.2f, false, COLOR_GREEN_1);

		ray = Ray{ray.origin + UnitY * spacing, ray.direction};
	}

	if(best_hit_results.hit)
	{
		vec3 hitpoint = point_from_detection(best_hit_results.ray, best_hit_results);
		ImDraw::AddPoint(IMHASH, hitpoint, 2.0, true, COLOR_RED_1);
	}

	return best_hit_results;
}

RaycastTest World::RaycastLights(const Ray ray) const
{
	float min_distance = MaxFloat;
	RaycastTest closest_hit{.hit = false, .distance = -1};

	const auto aabb_mesh = GeometryCatalogue.find("aabb")->second;

	int point_c = 0;
	for(auto& light : this->point_lights)
	{
		// subtract lightbulb model size from position
		auto position = light->position - vec3{0.1575, 0, 0.1575};
		auto aabb_model = translate(Mat4Identity, position);
		aabb_model = scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

		auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
		if(test.hit && test.distance < min_distance)
		{
			closest_hit = {true, test.distance, nullptr, point_c, "point"};
			min_distance = test.distance;
		}
		point_c++;
	}

	int spot_c = 0;
	for(auto& light : this->spot_lights)
	{
		// subtract lightbulb model size from position
		auto position = light->position - vec3{0.1575, 0, 0.1575};
		auto aabb_model = translate(Mat4Identity, position);
		aabb_model = scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

		const auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
		if(test.hit && test.distance < min_distance)
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
	for(const auto& entity : this->entities)
		manager->pool.FreeSlot(entity);

	this->entities.clear();

	// drops all lights
	for(const auto& light : this->point_lights)
		delete light;

	this->point_lights.clear();

	for(const auto& light : this->spot_lights)
		delete light;

	this->spot_lights.clear();

	for(const auto& light : this->directional_lights)
		delete light;

	this->directional_lights.clear();

	// drops all interactable entities
	for(const auto& interactable : this->interactables)
		delete interactable;

	this->interactables.clear();

	// drops all checkpoint entities
	for(const auto& checkpoint : this->checkpoints)
		delete checkpoint;

	this->checkpoints.clear();
}

void World::UpdateEntities() const
{
	for(const auto& entity : this->entities)
		entity->Update();
}
