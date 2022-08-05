#pragma once

// Global variable to control entity IDs
inline u64 Max_Entity_Id = 0;

enum SrEntityRelation {
   SrEntityRelation_TimerTarget    = 0,
   SrEntityRelation_TimerMarking   = 1
};

// Allows storing relationships between parsed entities to be set after parsing is done
struct DeferredEntityRelationBuffer {
   static constexpr int    size = 64;
   int                     count = 0;
   Entity*                 entities[size]{};
   u64                     deferred_entity_ids[size]{};
   SrEntityRelation        relations[size]{};
   u32                     aux_uint_buffer[size]{};
};


// Prototypes
bool load_scene_from_file(const std::string& scene_name, World* world);
Entity* parse_and_load_entity(
   Parser::Parse p, std::ifstream* reader, int& line_count, std::string path, DeferredEntityRelationBuffer* entity_relations
);
void parse_and_load_player_attribute(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path, Player* player);
void parse_and_load_light_source(Parser::Parse p, std::ifstream* reader, int& line_count,std::string path, World* world);
void parse_and_load_camera_settings(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path);
void parse_and_load_player_orientation(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path, Player* player);
bool load_player_attributes_from_file();
bool check_if_scene_exists();

ProgramConfig load_configs();
bool save_configs_to_file();

#include <iomanip>
#include <sr_load_player.h>
#include <sr_load_lights.h>
#include <sr_load_configs.h>
#include <sr_load_entity.h>

bool load_scene_from_file(const std::string& scene_name, World* world)
{
   std::string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   std::ifstream reader(path);

   if(!reader.is_open())
   {
      std::cout << "Cant load scene from file '" + path + "', path NOT FOUND \n";  
      return false;
   }

   // clears the current scene entity data
   world->clear(&Entity_Manager);

   // Gets a new world struct from scratch
   world->init();

   G_SCENE_INFO.camera = G_SCENE_INFO.views[0]; // sets to editor camera
   Entity_Manager.set_entity_registry(&world->entities);
   Entity_Manager.set_checkpoints_registry(&world->checkpoints);
   Entity_Manager.set_interactables_registry(&world->interactables);

   // Either creates new scene or resets current structures
   if(G_SCENE_INFO.active_scene == nullptr)
   {
      G_SCENE_INFO.active_scene  =  new Scene();
      G_SCENE_INFO.player        = new Player();
   }
   else
   {
      *G_SCENE_INFO.active_scene = Scene{};
      *G_SCENE_INFO.player       = Player{};
   }

   G_SCENE_INFO.player->entity_ptr = Entity_Manager.create_entity(
      PLAYER_NAME, "capsule", "model", "pink", "capsule", vec3(1));

   // creates deferred load buffer for associating entities after loading them all
   auto entity_relations = DeferredEntityRelationBuffer();

   // starts reading
   std::string line;
   Parser::Parse p{};
   int line_count = 0;

   // parses header
   parser_nextline(&reader, &line, &p);
   line_count++;

   p = parse_token(p);
   if(!p.hasToken)
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.")
   
   std::string next_entity_id_token = p.string_buffer;
   if(next_entity_id_token != "NEXT_ENTITY_ID")
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.")

   p = parse_whitespace(p);
   p = parse_symbol(p);
   if(!p.hasToken || p.cToken != '=')
      Quit_fatal("Missing '=' after NEXT_ENTITY_ID.")

   p = parse_whitespace(p);
   p = parse_u64(p);

   // ENTITY IDs related code
   bool recompute_next_entity_id = false;
   if(!p.hasToken)
      recompute_next_entity_id = true;
   else
      Entity_Manager.next_entity_id = p.u64Token;

   // -----------------------------------
   //           Parse entities
   // -----------------------------------
   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;

      p = parse_symbol(p);
      if(p.cToken == '#')
      {
         Entity* new_entity = parse_and_load_entity(p, &reader, line_count, path, &entity_relations);

         // set up collider
         new_entity->collider = *new_entity->collision_mesh;
         // puts entity into entities list and update geometric properties
         Entity_Manager.register_in_world_and_scene(new_entity);
      }
      else if(p.cToken == '@')
      {
         parse_and_load_player_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
      else if(p.cToken == '$')
      {
         parse_and_load_light_source(p, &reader, line_count, path, world);
      }
      else if(p.cToken == '*')
      {
         parse_and_load_camera_settings(p, &reader, line_count, path);
      }
      else if(p.cToken == '&')
      {
         parse_and_load_player_orientation(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
   }

   // -----------------------------------
   //          Post parse steps
   // -----------------------------------
   G_SCENE_INFO.player->update(world, true);
   CL_update_player_world_cells(G_SCENE_INFO.player, world);

   // connects entities using deferred load buffer
   For(entity_relations.count)
   {
      //@TODO: wtf is going on with 2 entity vars being declared here?
      Entity* entity                = entity_relations.entities[i];
      auto relation                 = entity_relations.relations[i];
      auto deferred_entity_id    = entity_relations.deferred_entity_ids[i];
      Entity* deferred_entity       = nullptr;

      Forj(world->entities.size())
      {  
         Entity* entity = world->entities[j];
         if(entity->id == deferred_entity_id)
         {
            deferred_entity = entity;
            break;
         }
      }

      if(deferred_entity == nullptr)
         Quit_fatal("Entity with id '" + std::to_string(deferred_entity_id) + "' not found to stablish a defined entity relationship.");

      switch(relation)
      {
         case SrEntityRelation_TimerTarget:
         {
            entity->timer_trigger_data.timer_target = deferred_entity;
            break;
         }

         case SrEntityRelation_TimerMarking:
         {
            u32 time_checkpoint = entity_relations.aux_uint_buffer[i];
            entity->timer_trigger_data.add_marking(deferred_entity, time_checkpoint);
            break;
         }
      }
   }


   // -----------------------------------
   //         Entity id bookkeeping
   // -----------------------------------

   // If missing NEXT_ENTITY_ID in scene header, recompute from collected Ids (If no entity has an ID yet, this will be 1)
   if(recompute_next_entity_id)
   {
      Entity_Manager.next_entity_id = Max_Entity_Id + 1;
   }

   // assign IDs to entities missing them starting from max current id
   For(world->entities.size())
   {
      auto entity = world->entities[i];
      if(entity->name != PLAYER_NAME && entity->id == -1)
      {
         entity->id = Entity_Manager.next_entity_id++;
      }
   }
   
   world->update_cells_in_use_list();

   G_SCENE_INFO.scene_name = scene_name;

   // save backup
   save_scene_to_file("backup", G_SCENE_INFO.player, world, true);

   return true;
} 



inline bool check_if_scene_exists(const std::string& scene_name)
{
   std::string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   std::ifstream reader(path);
   return reader.is_open();
}