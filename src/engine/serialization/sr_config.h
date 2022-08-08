#pragma once

struct Parser;
struct EntityManager;
struct World;
struct ProgramConfig;

//@TODO: This is not ideal (GlobalSceneInfo must die)

struct ConfigSerializer
{
   static inline GlobalSceneInfo* scene_info = nullptr;

   static void             parse_camera_settings(Parser& p);
   static ProgramConfig    load_configs();
   static bool             save(const ProgramConfig& config);
};
