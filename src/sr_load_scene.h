
// Global variable to control entity IDs
u64 Max_Entity_Id = 0;

struct DeferredEntityRelationBuffer {
   static const int size = 64;
   int count = 0;
   Entity* from[size];
   u64 to[size];
   std::string context[size];
};

// Prototypes
bool load_scene_from_file(std::string scene_name, WorldStruct* world);
Entity* parse_and_load_entity(
   Parser::Parse p, ifstream* reader, int& line_count, std::string path, DeferredEntityRelationBuffer* entity_relations
);
void parse_and_load_player_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
void parse_and_load_light_source(Parser::Parse p, ifstream* reader, int& line_count, string path);
void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_player_orientation(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
bool load_player_attributes_from_file();
bool check_if_scene_exists();

Entity* create_player_entity();
Player* create_player(Entity* player_entity);
ProgramConfig load_configs();
bool save_configs_to_file();

#include <iomanip>


#include <sr_load_player.h>
#include <sr_load_lights.h>
#include <sr_load_configs.h>
#include <sr_load_entity.h>

bool load_scene_from_file(std::string scene_name, WorldStruct* world)
{
   string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   ifstream reader(path);

   if(!reader.is_open())
   {
      cout << "Cant load scene from file '" + path + "', path NOT FOUND \n";  
      return false;
   }

   // clears the current scene entity data
   if(G_SCENE_INFO.active_scene != NULL)
      G_SCENE_INFO.active_scene->entities.clear();
      // clear buffers ?

   // Gets a new world struct from scratch
   World.init();

   // creates new scene
   // @todo: possibly leaking memory if switching between scenes.
   auto scene = new Scene();
   G_SCENE_INFO.active_scene = scene;
   G_SCENE_INFO.camera = G_SCENE_INFO.views[0];    // sets to editor camera
   Entity_Manager.set_entity_registry(&G_SCENE_INFO.active_scene->entities);
   Entity_Manager.set_checkpoints_registry(&G_SCENE_INFO.active_scene->checkpoints);
   Entity_Manager.set_interactables_registry(&G_SCENE_INFO.active_scene->interactables);

   // creates player
   auto player_entity = create_player_entity();
   auto player = create_player(player_entity);
   G_SCENE_INFO.player = player;

   // creates deferred load buffer for associating entities after loading them all
   auto entity_relations = DeferredEntityRelationBuffer();

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   // parses header
   parser_nextline(&reader, &line, &p);
   line_count++;

   p = parse_token(p);
   if(!p.hasToken)
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.");
   
   std::string next_entity_id_token = p.string_buffer;
   if(next_entity_id_token != "NEXT_ENTITY_ID")
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.");

   p = parse_whitespace(p);
   p = parse_symbol(p);
   if(!p.hasToken || p.cToken != '=')
      Quit_fatal("Missing '=' after NEXT_ENTITY_ID.");

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
         new_entity->collider       = *new_entity->collision_mesh;
         new_entity->collider.name  = new_entity->name + "-collider";
         new_entity->collider.setup_gl_data();

         // puts entity into entities list and update geometric properties
         Entity_Manager.register_in_world_and_scene(new_entity);
      }
      else if(p.cToken == '@')
      {
         parse_and_load_player_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
      else if(p.cToken == '$')
      {
         parse_and_load_light_source(p, &reader, line_count, path);
      }
      else if(p.cToken == '*')
      {
         parse_and_load_camera_settings(p, &reader, line_count, path);
      }
      else if(p.cToken == '&')
      {
         parse_and_load_player_orientation(p, &reader, line_count, path, player);
      }
   }

   // -----------------------------------
   //          Post parse steps
   // -----------------------------------

   // connects entities using deferred load buffer
   For(entity_relations.count)
   {
      Entity* from         = entity_relations.from[i];
      std::string context  = entity_relations.context[i];
      Entity* to_entity;

      bool found_to_entity = false;
      Forj(scene->entities.size())
      {  
         Entity* entity = scene->entities[j];
         if(entity->id == entity_relations.to[i])
         {
            to_entity = entity;
            found_to_entity = true;
            break;
         }
      }

      if(!found_to_entity)
      {
         Quit_fatal("Something weird happened. We tried to deferred load the relationship of '"
            + context + "' between (from) '" + from->name + "' and an entity with id '" 
            + to_string(entity_relations.to[i]) + "' but we couldn't find that entity inside the scene list.");
      }

      if(context == "timer_target")
      {
         from->timer_trigger_data.timer_target = to_entity;
         Entity_Manager.set_type(to_entity, EntityType_TimerTarget);

         // initializes data for triggers of time_attack_door
         //@todo should be any kind of time_attack_door, but ok
         if(to_entity->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
         {
            auto data = &from->timer_trigger_data;
            new(data) TimerTriggerData();        // because cpp unions...
            For(data->size)
            {
               //@todo we will, obviously, load these in when we serialize it to the file
               data->markings[i]             = nullptr;
               data->notification_mask[i]    = false;
               data->time_checkpoints[i]     = 0;
            }
         }
      }
   }


   // -----------------------------------
   //         Entity id bookkeeping
   // -----------------------------------

   // If misisng NEXT_ENTITY_ID in scene header, recompute from collected Ids (If no entity has an ID yet, this will be 1)
   if(recompute_next_entity_id)
   {
      Entity_Manager.next_entity_id = Max_Entity_Id + 1;
   }

   // assign IDs to entities missing them starting from max current id
   For(scene->entities.size())
   {
      auto entity = scene->entities[i];
      if(entity->name != PLAYER_NAME && entity->id == -1)
      {
         entity->id = Entity_Manager.next_entity_id++;
      }
   }
   
   world->update_cells_in_use_list();

   G_SCENE_INFO.scene_name = scene_name;

   // save backup
   save_scene_to_file("backup", player, true);

   return true;
} 



bool check_if_scene_exists(string scene_name)
{
   string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   ifstream reader(path);
   return reader.is_open();
}