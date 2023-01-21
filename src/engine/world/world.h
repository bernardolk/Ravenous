#pragma once

#include "engine/collision/primitives/bounding_box.h"

auto world_coords_to_cells(float x, float y, float z);
vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k);

// how many cells we have preallocated for the world
const static int W_CELLS_NUM_X = 10;
const static int W_CELLS_NUM_Y = 10;
const static int W_CELLS_NUM_Z = 10;

// how many cells are before and after the origin in each axis
const static int W_CELLS_OFFSET_X = W_CELLS_NUM_X / 2;
const static int W_CELLS_OFFSET_Y = W_CELLS_NUM_Y / 2;
const static int W_CELLS_OFFSET_Z = W_CELLS_NUM_Z / 2;

// how many meters the cell occupies in the world
const static float W_CELL_LEN_METERS = 50.0f;
// how many entities can coexists in a cell
const static int WORLD_CELL_CAPACITY = 150;

const static vec3 W_UPPER_BOUNDS_METERS = {
	W_CELLS_OFFSET_X * W_CELL_LEN_METERS,
	W_CELLS_OFFSET_Y * W_CELL_LEN_METERS,
	W_CELLS_OFFSET_Z * W_CELL_LEN_METERS
};

const static vec3 W_LOWER_BOUNDS_METERS = {
	-1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS,
	-1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS,
	-1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS
};

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
	CellUpdateStatus status{};
	std::string      message{};
	bool             entity_changed_cell = false;
};

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
	Entity*      entities[WORLD_CELL_CAPACITY]{};
	unsigned int count = 0;

	// logical coords
	int i = -1, j = -1, k = -1;

	// world coords / bounding box
	BoundingBox bounding_box{};

	void        init(int ii, int ji, int ki);
	void        remove(Entity* entity);
	CellUpdate  add(Entity* entity);
	void        defrag();
	std::string coords_str() const;
	vec3        coords() const;
	vec3        coords_meters() const;
	std::string coords_meters_str();
};

// -------------------
// COORDINATE METHODS
// -------------------

inline vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
	const float world_x = (static_cast<float>(i) - W_CELLS_OFFSET_X) * W_CELL_LEN_METERS;
	const float world_y = (static_cast<float>(j) - W_CELLS_OFFSET_Y) * W_CELL_LEN_METERS;
	const float world_z = (static_cast<float>(k) - W_CELLS_OFFSET_Z) * W_CELL_LEN_METERS;

	return vec3{world_x, world_y, world_z};
}


inline auto world_coords_to_cells(float x, float y, float z)
{
	struct
	{
		int i = -1, j = -1, k = -1;
	}       world_cell_coords;

	// if out of bounds return -1
	if(x < W_LOWER_BOUNDS_METERS.x || x > W_UPPER_BOUNDS_METERS.x ||
		y < W_LOWER_BOUNDS_METERS.y || y > W_UPPER_BOUNDS_METERS.y ||
		z < W_LOWER_BOUNDS_METERS.z || z > W_UPPER_BOUNDS_METERS.z)
	{
		world_cell_coords.i = -1;
		world_cell_coords.j = -1;
		world_cell_coords.k = -1;
		return world_cell_coords;
	}

	// int division to truncate float result to correct cell position
	world_cell_coords.i = (x + W_CELLS_OFFSET_X * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;
	world_cell_coords.j = (y + W_CELLS_OFFSET_Y * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;
	world_cell_coords.k = (z + W_CELLS_OFFSET_Z * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;

	return world_cell_coords;
}

inline auto world_coords_to_cells(vec3 position)
{
	return world_coords_to_cells(position.x, position.y, position.z);
}


// ----------------
// > WORLD
// ----------------
struct World
{

	// Entities lists
	std::vector<Entity*>           entities;
	std::vector<Entity*>           interactables;
	std::vector<Entity*>           checkpoints;
	std::vector<PointLight*>       point_lights;
	std::vector<SpotLight*>        spot_lights;
	std::vector<DirectionalLight*> directional_lights;

	Player* player = nullptr;

	float global_shininess = 17;
	float ambient_intensity = 0;
	vec3  ambient_light = vec3(1);

	WorldCell  cells[W_CELLS_NUM_X][W_CELLS_NUM_Y][W_CELLS_NUM_Z];
	WorldCell* cells_in_use[W_CELLS_NUM_X * W_CELLS_NUM_Y * W_CELLS_NUM_Z]{};
	int        cells_in_use_count = 0;

public:
	World();

	void init();
	void update_cells_in_use_list();
	void update_entities() const;
	void clear(const EntityManager* manager);

	RaycastTest raycast(
		Ray           ray,
		RayCastType   test_type,
		const Entity* skip = nullptr,
		float         max_distance = MAX_FLOAT
	) const;
	RaycastTest raycast(
		Ray           ray,
		const Entity* skip = nullptr,
		float         max_distance = MAX_FLOAT
	) const;
	RaycastTest linear_raycast_array(Ray first_ray, int qty, float spacing) const;
	RaycastTest raycast_lights(Ray ray) const;
	CellUpdate  update_entity_world_cells(Entity* entity);
};
