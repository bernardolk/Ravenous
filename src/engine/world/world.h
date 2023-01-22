#pragma once

#include "engine/collision/primitives/bounding_box.h"

auto world_coords_to_cells(float x, float y, float z);
vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k);

// how many cells we have preallocated for the world
constexpr static int WCellsNumX = 10;
constexpr static int WCellsNumY = 10;
constexpr static int WCellsNumZ = 10;

// how many cells are before and after the origin in each axis
constexpr static int WCellsOffsetX = WCellsNumX / 2;
constexpr static int WCellsOffsetY = WCellsNumY / 2;
constexpr static int WCellsOffsetZ = WCellsNumZ / 2;

// how many meters the cell occupies in the world
constexpr static float WCellLenMeters = 50.0f;
// how many entities can coexists in a cell
constexpr static int WorldCellCapacity = 150;

const static vec3 WUpperBoundsMeters = {
WCellsOffsetX * WCellLenMeters,
WCellsOffsetY * WCellLenMeters,
WCellsOffsetZ * WCellLenMeters
};

const static vec3 WLowerBoundsMeters = {
-1.0 * WCellsOffsetX * WCellLenMeters,
-1.0 * WCellsOffsetX * WCellLenMeters,
-1.0 * WCellsOffsetX * WCellLenMeters
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
	std::string message{};
	bool entity_changed_cell = false;
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
struct T_EntityManager;

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

inline vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
	const float world_x = (static_cast<float>(i) - WCellsOffsetX) * WCellLenMeters;
	const float world_y = (static_cast<float>(j) - WCellsOffsetY) * WCellLenMeters;
	const float world_z = (static_cast<float>(k) - WCellsOffsetZ) * WCellLenMeters;

	return vec3{world_x, world_y, world_z};
}


inline auto world_coords_to_cells(float x, float y, float z)
{
	struct
	{
		int i = -1, j = -1, k = -1;
	} world_cell_coords;

	// if out of bounds return -1
	if(x < WLowerBoundsMeters.x || x > WUpperBoundsMeters.x ||
		y < WLowerBoundsMeters.y || y > WUpperBoundsMeters.y ||
		z < WLowerBoundsMeters.z || z > WUpperBoundsMeters.z)
	{
		world_cell_coords.i = -1;
		world_cell_coords.j = -1;
		world_cell_coords.k = -1;
		return world_cell_coords;
	}

	// int division to truncate float result to correct cell position
	world_cell_coords.i = (x + WCellsOffsetX * WCellLenMeters) / WCellLenMeters;
	world_cell_coords.j = (y + WCellsOffsetY * WCellLenMeters) / WCellLenMeters;
	world_cell_coords.k = (z + WCellsOffsetZ * WCellLenMeters) / WCellLenMeters;

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

	WorldCell cells[WCellsNumX][WCellsNumY][WCellsNumZ];
	WorldCell* cells_in_use[WCellsNumX * WCellsNumY * WCellsNumZ]{};
	int cells_in_use_count = 0;

public:
	World();

	void Init();
	void UpdateCellsInUseList();
	void UpdateEntities() const;
	void Clear(const T_EntityManager* manager);

	RaycastTest Raycast(Ray ray, RayCastType test_type, const Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest Raycast(Ray ray, const Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest LinearRaycastArray(Ray first_ray, int qty, float spacing) const;
	RaycastTest RaycastLights(Ray ray) const;
	CellUpdate UpdateEntityWorldCells(Entity* entity);
};
