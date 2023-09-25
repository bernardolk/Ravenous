#pragma once

#include "engine/core/core.h"


// Global variable to control entity IDs
inline static uint64 MaxEntityId = 0;

struct WorldSerializer
{
	static inline RWorld* World = nullptr;
	// static inline EntityManager* manager = nullptr;

	static bool LoadFromFile(const string& Filename);
	static bool CheckIfSceneExists(const string& SceneName);
	static bool SaveToFile(const string& NewFilename, bool DoCopy);
	static bool SaveToFile();
};
