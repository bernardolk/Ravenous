#pragma once

struct Entity;
struct World;
struct EntityManager;

// Global variable to control entity IDs
inline static u64 Max_Entity_Id = 0;

struct WorldSerializer
{
    static bool load_from_file(const std::string& filename, World& world, EntityManager& entity_manager);
    static bool save_to_file(const std::string& filename, World& world);

    static void _parse_entity();
    static void _parse_light();
    static void _parse_camera_settings();
    static void _parse_player_settings();
};