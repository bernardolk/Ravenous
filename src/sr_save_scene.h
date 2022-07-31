
bool save_scene_to_file(std::string scene_name, Player* player, World* world, bool do_copy)
{
   bool was_renamed = scene_name.length() > 0;
   if(!was_renamed)
      scene_name = G_SCENE_INFO.scene_name;

   if(do_copy && !was_renamed)
   {
      std::cout << "please provide a name for the copy.\n";
      return false;
   }

  std::string path = SCENES_FOLDER_PATH + scene_name + ".txt";

   std::ofstream writer(path);
   if(!writer.is_open())
   {
      std::cout << "Saving scene failed.\n";
      return false;
   }

   writer << std::fixed << std::setprecision(4);

   writer << "NEXT_ENTITY_ID = " << Entity_Manager.next_entity_id << "\n";

   // write camera settings to file
   auto camera = G_SCENE_INFO.views[0];
   writer << "*" 
      << camera->Position.x << " "
      << camera->Position.y << " "
      << camera->Position.z << "  "
      << camera->Front.x << " "
      << camera->Front.y << " "
      << camera->Front.z << "\n";

   // write player attributes to file
   writer << "@player_position = " 
               << player->entity_ptr->position.x << " " 
               << player->entity_ptr->position.y << " "
               << player->entity_ptr->position.z << "\n";
   writer << "@player_initial_velocity = "
               << player->initial_velocity.x << " " 
               << player->initial_velocity.y << " "
               << player->initial_velocity.z << "\n";

   if(player->player_state == PLAYER_STATE_STANDING)
      writer << "@player_state = " << PLAYER_STATE_STANDING << "\n"; 
   else
      writer << "@player_state = " << player->initial_player_state << "\n"; 

   writer << "@player_fall_acceleration = " << player->fall_acceleration << "\n";
   auto fps_cam = G_SCENE_INFO.views[FPS_CAM];
   writer << "&player_orientation = "
      << fps_cam->Front.x << " "
      << fps_cam->Front.y << " "
      << fps_cam->Front.z << "\n"; 

   // write light sources POINT
   for(int i = 0; i < world->point_lights.size(); i++)
   {
      auto light = *world->point_lights[i];

      writer << "\n$point\n"
            << "position "
            << light.position.x << " "
            << light.position.y << " "
            << light.position.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n"
            << "constant "
            << light.intensity_constant << "\n"
            << "linear "
            << light.intensity_linear << "\n"
            << "quadratic "
            << light.intensity_quadratic << "\n";
   }

   // write light sources SPOT
   for(int i = 0; i < world->spot_lights.size(); i++)
   {
      auto light = *world->spot_lights[i];

      writer << "\n$spot\n"
            << "position "
            << light.position.x << " "
            << light.position.y << " "
            << light.position.z << "\n"
            << "direction "
            << light.direction.x << " "
            << light.direction.y << " "
            << light.direction.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n"
            << "innercone "
            << light.innercone << "\n"
            << "outercone "
            << light.outercone << "\n"
            << "constant "
            << light.intensity_constant << "\n"
            << "linear "
            << light.intensity_linear << "\n"
            << "quadratic "
            << light.intensity_quadratic << "\n";
   }

   // write light sources DIRECTIONAL
   for(int i = 0; i < world->directional_lights.size(); i++)
   {
      auto light = *world->directional_lights[i];

      writer << "\n$directional\n"
            << "direction "
            << light.direction.x << " "
            << light.direction.y << " "
            << light.direction.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n";
   }

   for(int i = 0; i < world->entities.size(); i++) 
   {
      Entity* entity = world->entities[i];
      if(entity->name == "Player")
         continue;

      writer << "\n#" << entity->name << "\n";
      writer << "id " << entity->id << "\n";
      writer << "position " 
               << entity->position.x << " "
               << entity->position.y << " "
               << entity->position.z << "\n";
      writer << "rotation " 
               << entity->rotation.x << " "
               << entity->rotation.y << " "
               << entity->rotation.z << "\n";
      writer << "scale " 
               << entity->scale.x << " "
               << entity->scale.y << " "
               << entity->scale.z << "\n";
      writer << "mesh " << entity->mesh->name << "\n";
      writer << "shader " << entity->shader->name;

      // shader: If entity is using tiled texture fragment shader, also writes number of tiles since we can change it through the editor
      if(entity->flags & EntityFlags_RenderTiledTexture)
         For(6) {
            writer << " " << entity->uv_tile_wrap[i];
         }

      writer << "\n";

      int textures =  entity->textures.size();
      For(textures)
      {
         Texture texture = entity->textures[i];
         if(texture.type == "texture_diffuse")
            writer << "texture " << texture.name << "\n";
      }

      if(entity->flags & EntityFlags_RenderWireframe)
         writer << "hidden\n";

      switch(entity->type)
      {
         case EntityType_Static:
         {
            writer << "type static\n";
            break;
         }

         case EntityType_Checkpoint:
         {
            writer << "type checkpoint\n";
            writer << "trigger " 
               << entity->trigger_scale.x << " "
               << entity->trigger_scale.y << " "
               << entity->trigger_scale.z << "\n";
            break;
         }

         case EntityType_TimerTrigger:
         {
            writer << "type timer_trigger\n";
            writer << "trigger " 
               << entity->trigger_scale.x << " "
               << entity->trigger_scale.y << " "
               << entity->trigger_scale.z << "\n";
            if(entity->timer_trigger_data.timer_target != nullptr)
               writer << "timer_target " << entity->timer_trigger_data.timer_target->id << "\n";
            writer << "timer_duration " << entity->timer_trigger_data.timer_duration << "\n";

            For(entity->timer_trigger_data.size)
            {
               auto marking            = entity->timer_trigger_data.markings[i];
               u32  time_checkpoint    = entity->timer_trigger_data.time_checkpoints[i];
               if(marking != nullptr)
                  writer << "timer_marking " << marking->id << " " << time_checkpoint << "\n";
            }

            break;
         }

         case EntityType_TimerTarget:
         {
            writer << "type timer_target\n";
            writer << "timer_target_type " << entity->timer_target_data.timer_target_type << "\n";

            if(entity->timer_target_data.timer_start_animation != 0)
               writer << "timer_start_animation " << entity->timer_target_data.timer_start_animation << "\n";

            if(entity->timer_target_data.timer_stop_animation != 0)
               writer << "timer_stop_animation " << entity->timer_target_data.timer_stop_animation << "\n";

            break;
         }

         case EntityType_TimerMarking:
         {
            writer << "type timer_marking\n";
            writer << "timer_marking_color_on "
               << entity->timer_marking_data.color_on.x << " "
               << entity->timer_marking_data.color_on.y << " "
               << entity->timer_marking_data.color_on.z << "\n";

            writer << "timer_marking_color_off "
               << entity->timer_marking_data.color_off.x << " "
               << entity->timer_marking_data.color_off.y << " "
               << entity->timer_marking_data.color_off.z << "\n";

            break;
         }
      }

      if(entity->slidable)
      {
         writer << "slidable \n";
      }
   }

   writer.close();

   if(do_copy)
      log(LOG_INFO, "Scene copy saved succesfully as '" + scene_name + ".txt'");
   else if(was_renamed)
   {
      log(LOG_INFO, "Scene saved succesfully as '" + scene_name + ".txt' (now editing it)");
      G_SCENE_INFO.scene_name = scene_name;
   }
   else
      log(LOG_INFO, "Scene saved succesfully.");

   return true;
}