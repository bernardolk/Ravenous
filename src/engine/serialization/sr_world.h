#pragma once

struct Entity;
struct World;
struct T_EntityManager;
struct EntitySerializer;
struct PlayerSerializer;
struct LightSerializer;

// Global variable to control entity IDs
inline static u64 Max_Entity_Id = 0;

struct WorldSerializer
{
	static inline World* world = nullptr;
	static inline T_EntityManager* manager = nullptr;

	static bool load_from_file(const std::string& filename);
	static bool check_if_scene_exists(const std::string& scene_name);
	static bool save_to_file(const std::string& new_filename, bool do_copy);
	static bool save_to_file();
};
