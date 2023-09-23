#pragma once

struct Parser;
struct EntityManager;
struct World;
struct ProgramConfig;

//@TODO: This is not ideal (GlobalSceneInfo must die)

struct ConfigSerializer
{
	static void ParseCameraSettings(Parser& p);
	static void LoadGlobalConfigs();
	static bool Save(const ProgramConfig& config);
};