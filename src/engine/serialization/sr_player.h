#pragma once


struct Parser;
struct EntityManager;
struct World;

struct PlayerSerializer
{
   static inline World* world = nullptr;
   
   static void parse_attribute      (Parser& p);
   static void parse_orientation    (Parser& p);
   static void save                 (std::ofstream& writer);
};




