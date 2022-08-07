#pragma once

struct Entity;
struct World;
struct EntityManager;
struct EntitySerializer;
struct PlayerSerializer;
struct LightSerializer;

// Global variable to control entity IDs
inline static u64 Max_Entity_Id = 0;

struct WorldSerializer
{
   static World& world;
   static EntityManager& manager;
   static GlobalSceneInfo& config;

   static bool load_from_file(const std::string& filename);
   static bool save_to_file(const std::string& filename);
   
   static void _parse_entity(Parser& p);
   static void _parse_light(Parser& p);
   static void _parse_camera_settings(Parser& p);
   static void _parse_player_attributes(Parser& p);
   static void _parse_player_orientation(Parser& p);
};