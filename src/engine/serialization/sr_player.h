#pragma once


struct Parser;
struct EntityManager;
struct World;

struct PlayerSerializer
{
   static World&         world;
   static EntityManager& manager;
   
   static void parse_attribute(Parser& p);
   static void parse_orientation(Parser& p);
   static void save();
};




