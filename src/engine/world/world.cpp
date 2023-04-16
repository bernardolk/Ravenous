#include "engine/world/world.h"

#include "engine/catalogues.h"
#include "engine/entities/e_entity.h"
#include "engine/entities/lights.h"
#include "engine/render/im_render.h"
#include "engine/utils/utils.h"
#include "game/entities/player.h"

T_World::T_World()
{
	auto* active_chunk = chunks.GetAt(0);
	active_chunks.push_back(active_chunk);
	chunks_map[{0,0,0}] = active_chunk;
}

void T_World::Update()
{
	UpdateTransforms();
	UpdateTraits();
}

void T_World::UpdateTransforms()
{
	auto entity_iter = GetEntityIterator();
	while (auto* entity = entity_iter())
	{
		entity->Update();	
	}
}

void T_World::UpdateTraits()
{
	auto* entity_traits_manager = EntityTraitsManager::Get();
	for (TraitID trait_id : entity_traits_manager->entity_traits)
	{
		for (auto* chunk: active_chunks)
		{
			chunk->InvokeTraitUpdateOnAllTypes(trait_id);
		} 
	}	
}

Iterator<WorldChunk> T_World::GetChunkIterator()
{
	return chunks.GetIterator();
}

WorldEntityIterator T_World::GetEntityIterator()
{
	return WorldEntityIterator();
}

WorldChunkEntityIterator WorldChunk::GetIterator()
{
	return WorldChunkEntityIterator(this);
}

void WorldChunk::RemoveEntity(E_Entity* entity_to_delete)
{
	// TODO: We are not dealing with actually removing entities yet, we only mark them so they can be skipped/overwritten.
	entity_to_delete->deleted = true;
}


RaycastTest T_World::Raycast(const Ray ray, const RayCastType test_type, const E_Entity* skip, const float max_distance) const
{
	//@TODO: This should first test ray against world cells, then get the list of entities from these world cells to test against 

	float min_distance = MaxFloat;
	RaycastTest closest_hit{false, -1};

	auto entity_iterator = GetEntityIterator();
	while (auto* entity = entity_iterator())
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
}


RaycastTest T_World::Raycast(const Ray ray, const E_Entity* skip, const float max_distance) const
{
	return this->Raycast(ray, RayCast_TestOnlyFromOutsideIn, skip, max_distance);
}


RaycastTest T_World::LinearRaycastArray(const Ray first_ray, int qty, float spacing) const
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
		auto test = this->Raycast(ray, RayCast_TestOnlyFromOutsideIn, player, player->grab_reach);
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
}

RaycastTest T_World::RaycastLights(const Ray ray) const
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

CellUpdate T_World::UpdateEntityWorldCells(E_Entity* entity)
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
		entity->world_chunks_count == 0 ||
		entity->world_chunks[0]->i != i0 ||
		entity->world_chunks[0]->j != j0 ||
		entity->world_chunks[0]->k != k0 ||
		entity->world_chunks[entity->world_chunks_count - 1]->i != i1 ||
		entity->world_chunks[entity->world_chunks_count - 1]->j != j1 ||
		entity->world_chunks[entity->world_chunks_count - 1]->k != k1;

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
	for (int i = 0; i < entity->world_chunks_count; i++)
	{
		entity->world_chunks[i]->RemoveEntity(entity);
	}
	entity->world_chunks_count = 0;

	auto origin_chunk =  WorldCoordsToCells(entity->position);

	// Add entity to all world cells
	for (i32 i = i0; i <= i1; i++)
	{
		for (i32 j = j0; j <= j1; j++)
		{
			for (i32 k = k0; k <= k1; k++)
			{
				auto chunk_it = this->chunks_map.find({i,j,k});
				if (chunk_it == chunks_map.end())
					return CellUpdate{CellUpdate_OUT_OF_BOUNDS, "Coordinates not found in chunks_map.", true};
				
				auto* chunk = chunk_it->second;
				
				if (origin_chunk != vec3(i,j,k))
					chunk->AddVisitor(entity);
				
				entity->world_chunks.push_back(chunk);
			}
		}
	}

	return CellUpdate{CellUpdate_OK, "", true};
}

bool WorldChunk::AddVisitor(E_Entity* entity)
{
	visitors.push_back(entity);
	entity->visitor_state = VisitorState(true, vec3(i,j,k), this);
	return true;
}

bool WorldChunk::RemoveVisitor(E_Entity* entity)
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

// TODO: Move these elsewhere
void SetEntityDefaultAssets(E_Entity* entity)
{
	// Trusting type defaults
	EntityAttributes attrs;

	auto [
		_textures,
		_texture_count,
		_mesh,
		_collision_mesh,
		_shader] = FindEntityAssetsInCatalogue(attrs.mesh, attrs.collision_mesh, attrs.shader, attrs.texture);

	entity->name = attrs.name;
	entity->shader = _shader;
	entity->mesh = _mesh;
	entity->scale = attrs.scale;
	entity->collision_mesh = _collision_mesh;
	entity->collider = *_collision_mesh;

	For(_texture_count)
		entity->textures.push_back(_textures[i]);
}
	
void SetEntityAssets(E_Entity* entity, EntityAttributes attrs)
{
	auto [
		_textures,
		_texture_count,
		_mesh,
		_collision_mesh,
		_shader] = FindEntityAssetsInCatalogue(attrs.mesh, attrs.collision_mesh, attrs.shader, attrs.texture);

	entity->name = attrs.name;
	entity->shader = _shader;
	entity->mesh = _mesh;
	entity->scale = attrs.scale;
	entity->collision_mesh = _collision_mesh;
	entity->collider = *_collision_mesh;

	For(_texture_count)
		entity->textures.push_back(_textures[i]);
}
