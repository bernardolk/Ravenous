#pragma once

#include "engine/core/core.h"


// Global variable to control entity IDs
inline static u64 Max_Entity_Id = 0;

struct WorldSerializer
{
	static inline World* world = nullptr;
	static inline EntityManager* manager = nullptr;

	static bool LoadFromFile(const std::string& filename);
	static bool CheckIfSceneExists(const std::string& scene_name);
	static bool SaveToFile(const std::string& new_filename, bool do_copy);
	static bool SaveToFile();
};
