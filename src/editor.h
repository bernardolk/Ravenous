
namespace Editor
{

struct EntityPanelContext {
   Entity* entity = nullptr;
   vec3 original_position = vec3(0);
   vec3 original_scale = vec3(0);
   float original_rotation = 0;
   bool active = false;
   char rename_buffer[100];
};

struct EditorContext {
   ImGuiStyle* imStyle;
   EntityPanelContext entity_panel;

   bool move_entity_with_mouse = false;
   bool mouse_click = false;
   Entity* last_selected_entity = nullptr;
   vec3 last_selected_entity_original_position;
} Context;

void check_selection_to_open_panel();
void render();
void select_entity(Entity* entity);
void render_entity_panel(EntityPanelContext* panel_context);
void start_frame();
void end_frame();
void initialize();
void terminate();
void update();
void debug_entities();
void immediate_draw_aabb_boundaries(Entity* entity);
void move_entity_with_mouse(Entity* entity);
void handle_input_flags(InputFlags flags, Player* &player);
void undo_entity_panel_changes();
void undo_selected_entity_move_changes();
void deselect_entity();
void set_entity_panel(Entity* entity);


void update()
{
   // ACTIVATE this for editor mode entity debugging code
   
   //debug_entities();

   // respond to mouse if necessary
   if(Context.move_entity_with_mouse)
   {
      if(Context.mouse_click)
      {
         deselect_entity();
      }
      else
      {
         move_entity_with_mouse(Context.last_selected_entity);
      }
   }

   Context.mouse_click = false;
}

void move_entity_with_mouse(Entity* entity)
{
   Ray ray = cast_pickray();
   float distance = 0;

   auto t1 = Triangle{
      vec3{entity->position.x - 50, entity->position.y, entity->position.z - 50},
      vec3{entity->position.x + 50, entity->position.y, entity->position.z - 50}, 
      vec3{entity->position.x + 50, entity->position.y, entity->position.z + 50}
   };

   RaycastTest test1 = test_ray_against_triangle(ray, t1);
   if(test1.hit)
      distance = test1.distance;

   auto t2 = Triangle{
      vec3{entity->position.x - 50, entity->position.y, entity->position.z - 50},
      vec3{entity->position.x - 50, entity->position.y, entity->position.z + 50}, 
      vec3{entity->position.x + 50, entity->position.y, entity->position.z + 50}
   };

   RaycastTest test2 = test_ray_against_triangle(ray, t2);
   if(test2.hit)
      distance = test2.distance;

   if(distance != 0)
   {
      entity->position.x = ray.origin.x + ray.direction.x * distance;
      entity->position.z = ray.origin.z + ray.direction.z * distance;
   }
   else
   {
      cout << "warning: can't find plane to place entity!\n";
   }
}

void debug_entities()
{
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;

      if(entity->name == "small upper platform")
         immediate_draw_aabb_boundaries(entity);
   }
}

void immediate_draw_aabb_boundaries(Entity* entity)
{
   auto temp_col = entity->collision_geometry.aabb;
   G_IMMEDIATE_DRAW.add(
      vector<Vertex>{
         Vertex{entity->position},
         Vertex{entity->position + vec3(0, temp_col.length_y, 0)},

         Vertex{entity->position + vec3(temp_col.length_x, 0, 0)},
         Vertex{entity->position + vec3(temp_col.length_x, temp_col.length_y, 0)},

         Vertex{entity->position + vec3(0, 0, temp_col.length_z)},
         Vertex{entity->position + vec3(0, temp_col.length_y, temp_col.length_z)},

         Vertex{entity->position + vec3(temp_col.length_x, 0, temp_col.length_z)},
         Vertex{entity->position + vec3(temp_col.length_x, temp_col.length_y, temp_col.length_z)}
      },
      GL_POINTS
   );
}


void check_selection_to_open_panel()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
   {
      set_entity_panel(test.entity);
   }
}

void set_entity_panel(Entity* entity)
{
   Context.entity_panel.entity = entity;
   Context.entity_panel.active = true;
   Context.entity_panel.original_position = vec3{
      entity->position
   };
   Context.entity_panel.original_scale = vec3{
      entity->scale
   };
   Context.entity_panel.original_rotation =
      entity->rotation.y;
}


void check_selection_to_move_entity()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
   {
      select_entity(test.entity);
   }
}


void render()
{
   if(Context.entity_panel.active)
      render_entity_panel(&Context.entity_panel);
   else
      Context.entity_panel.rename_buffer[0] = 0;

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void select_entity(Entity* entity)
{
   Context.move_entity_with_mouse = true;
   Context.last_selected_entity = entity;
   Context.last_selected_entity_original_position = entity->position;
}

void deselect_entity()
{
   Context.move_entity_with_mouse = false;
}


void undo_entity_panel_changes()
{
   auto entity = Context.entity_panel.entity;
   entity->position = Context.entity_panel.original_position;
   entity->scale = Context.entity_panel.original_scale;
   entity->rotate_y(Context.entity_panel.original_rotation - entity->rotation.y);
}

void undo_selected_entity_move_changes()
{
   auto entity = Context.last_selected_entity;
   entity->position = Context.last_selected_entity_original_position;
   deselect_entity();
}


void render_entity_panel(EntityPanelContext* panel_context)
{
   auto entity = panel_context->entity;
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("Entity Panel", &panel_context->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text(entity->name.c_str());
   
   //rename
   if(ImGui::InputText("rename", &panel_context->rename_buffer[0], 100))
      entity->name = panel_context->rename_buffer;

   // position
   ImGui::SliderFloat(
      "x",
      &panel_context->entity->position.x,
      panel_context->original_position.x - 4,
      panel_context->original_position.x + 4
   );
   ImGui::SliderFloat(
      "y",
      &panel_context->entity->position.y,
      panel_context->original_position.y - 4,
      panel_context->original_position.y + 4
   );
   ImGui::SliderFloat(
      "z", 
      &panel_context->entity->position.z, 
      panel_context->original_position.z - 4, 
      panel_context->original_position.z + 4
   );

   // rotation
   float rotation = panel_context->entity->rotation.y;
   if(ImGui::InputFloat("rot y", &rotation, 90))
      Context.entity_panel.entity->rotate_y(rotation - panel_context->entity->rotation.y);

   // scale
   auto scale = vec3{panel_context->entity->scale};
   vec3 min_scales {
      panel_context->original_scale.x - 4,
      panel_context->original_scale.y - 4,
      panel_context->original_scale.z - 4
   };
   if(ImGui::SliderFloat(
      "scale x",
      &scale.x,
      min_scales.x < 0 ? 0: min_scales.x,
      panel_context->original_scale.x + 4
   ) ||
   ImGui::SliderFloat(
      "scale y",
      &scale.y,
      min_scales.y < 0 ? 0: min_scales.y,
      panel_context->original_scale.y + 4
   ) ||
   ImGui::SliderFloat(
      "scale z", 
      &scale.z,
      min_scales.z < 0 ? 0: min_scales.z,
      panel_context->original_scale.z + 4
   ))
   {
      Context.entity_panel.entity->set_scale(scale);
   }
   
   // show boundaries
   // bool show_boundaries = false;
   // ImGui::Checkbox("Show collision geometry boundaries", &show_boundaries);
   // if(show_boundaries)
   //    immediate_draw_aabb_boundaries(entity);

   bool duplicate = false;
   ImGui::Checkbox("Duplicate", &duplicate);
   if(duplicate)
   {
      auto new_entity = copy_entity(entity);
      new_entity->name += " copy";
      bool already_exists = get_entity_position(G_SCENE_INFO.active_scene, new_entity) > -1;
      if(already_exists) new_entity->name += "(1)";
      G_SCENE_INFO.active_scene->entities.push_back(new_entity);
      select_entity(new_entity);
      set_entity_panel(new_entity);
   }

   bool erase = false;
   ImGui::Checkbox("Erase", &erase);
   if(erase)
   {
      auto& list = G_SCENE_INFO.active_scene->entities;
      int index = get_entity_position(G_SCENE_INFO.active_scene, entity);
      list.erase(list.begin() + index);
      Context.entity_panel.active = false;
   }

   ImGui::End();
}

void start_frame()
{
   ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void end_frame()
{
	ImGui::EndFrame();
}

void terminate()
{
   ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void initialize()
{
   const char* glsl_version = "#version 330";
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(G_DISPLAY_INFO.window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();
	Context.imStyle = &ImGui::GetStyle();
	Context.imStyle->WindowRounding = 1.0f;
}


void handle_input_flags(InputFlags flags, Player* &player)
{
   if(ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
      return;

   if(flags.key_press & KEY_LEFT_CTRL && pressed_once(flags, KEY_Z))
   {
      if(Context.entity_panel.active)
         undo_entity_panel_changes();
      else if(Context.last_selected_entity != nullptr)
         undo_selected_entity_move_changes();
   }

   if(pressed_once(flags, KEY_T))
   {  // toggle camera type
      if (G_SCENE_INFO.camera->type == FREE_ROAM)
         set_camera_to_third_person(G_SCENE_INFO.camera, player);
      else if (G_SCENE_INFO.camera->type == THIRD_PERSON)
         set_camera_to_free_roam(G_SCENE_INFO.camera);
   }
   // click selection
   if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK && flags.key_press & KEY_LEFT_CTRL)
   {
      Editor::check_selection_to_open_panel();
   }
   else if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK && flags.key_press & KEY_G)
   {
      Editor::check_selection_to_move_entity();
   }
   else if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK)
   {
      // deselection
      Context.mouse_click = true;
   }

   if(pressed_once(flags, KEY_GRAVE_TICK))
   { 
      start_console_mode();
   }
   if(pressed_once(flags, KEY_C))
   {
      // moves player to camera position
      player->entity_ptr->position = G_SCENE_INFO.camera->Position + G_SCENE_INFO.camera->Front * 3.0f;
      camera_look_at(G_SCENE_INFO.views[1], G_SCENE_INFO.camera->Front, false);
      player->player_state = PLAYER_STATE_FALLING;
      player->entity_ptr->velocity = vec3(0, 0, 0);
   }
   
   // @TODO: this sucks
   float camera_speed = 
      G_SCENE_INFO.camera->type == THIRD_PERSON ?
      player->speed * G_FRAME_INFO.delta_time * G_FRAME_INFO.time_step:
      G_FRAME_INFO.delta_time * G_SCENE_INFO.camera->Acceleration;

   if(flags.key_press & KEY_LEFT_SHIFT)
   {
      camera_speed = camera_speed * 2;
   }
   if(flags.key_press & KEY_LEFT_CTRL)
   {
      camera_speed = camera_speed / 2;
   }
   if(flags.key_press & KEY_W)
   {
      G_SCENE_INFO.camera->Position += camera_speed * G_SCENE_INFO.camera->Front;
   }
   if(flags.key_press & KEY_A)
   {
      // @TODO: this sucks too
      if(G_SCENE_INFO.camera->type == FREE_ROAM)
         G_SCENE_INFO.camera->Position -= camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      else if(G_SCENE_INFO.camera->type == THIRD_PERSON)
         G_SCENE_INFO.camera->orbital_angle -= 0.025;
   }
   if(flags.key_press & KEY_S)
   {
      G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Front;
   }
   if(flags.key_press & KEY_D)
   {
      if(G_SCENE_INFO.camera->type == FREE_ROAM)
         G_SCENE_INFO.camera->Position += camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      else if(G_SCENE_INFO.camera->type == THIRD_PERSON)
         G_SCENE_INFO.camera->orbital_angle += 0.025;
   }
   if(flags.key_press & KEY_Q)
   {
      G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Up;
   }
      if(flags.key_press & KEY_E)
   {
      G_SCENE_INFO.camera->Position += camera_speed * G_SCENE_INFO.camera->Up;
   }
   if(flags.key_press & KEY_O)
   {
      camera_look_at(G_SCENE_INFO.camera, vec3(0.0f, 0.0f, 0.0f), true);
   }
   if(player->player_state == PLAYER_STATE_STANDING)
   {
      // resets velocity
      player->entity_ptr->velocity = vec3(0); 

      if ((flags.key_press & KEY_UP && G_SCENE_INFO.camera->type == FREE_ROAM) || 
            (flags.key_press & KEY_W && G_SCENE_INFO.camera->type == THIRD_PERSON))
      {
         player->entity_ptr->velocity += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if ((flags.key_press & KEY_DOWN && G_SCENE_INFO.camera->type == FREE_ROAM) ||
            (flags.key_press & KEY_S && G_SCENE_INFO.camera->type == THIRD_PERSON))
      {
         player->entity_ptr->velocity -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if (flags.key_press & KEY_LEFT)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity -= vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      if (flags.key_press & KEY_RIGHT)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity += vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      // because above we sum all combos of keys pressed, here we normalize the direction and give the movement intensity
      if(glm::length2(player->entity_ptr->velocity) > 0)
      {
         float player_frame_speed = player->speed;
         if(flags.key_press & KEY_LEFT_SHIFT)  // PLAYER DASH
            player_frame_speed *= 2;

         player->entity_ptr->velocity = player_frame_speed * glm::normalize(player->entity_ptr->velocity);
      }
      if (flags.key_press & KEY_SPACE) 
      {
         player->player_state = PLAYER_STATE_JUMPING;
         player->height_before_fall = player->entity_ptr->position.y;
         player->entity_ptr->velocity.y = player->jump_initial_speed;
      }
   }
   else if(player->player_state == PLAYER_STATE_SLIDING)
   {
      auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
      player->entity_ptr->velocity = player->slide_speed * collision_geom.tangent;

      if (flags.key_press & KEY_LEFT)
      {
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, -12.0f, collision_geom.normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_RIGHT)
      {
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, 12.0f, collision_geom.normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_SPACE)
      {
            player->player_state = PLAYER_STATE_JUMPING;
            auto col_geometry = player->standing_entity_ptr->collision_geometry.slope;
            float x = col_geometry.normal.x > 0 ? 1 : col_geometry.normal.x == 0 ? 0 : -1;
            float z = col_geometry.normal.z > 0 ? 1 : col_geometry.normal.z == 0 ? 0 : -1;
            auto jump_vec = glm::normalize(vec3(x, 1, z));
            player->entity_ptr->velocity = player->jump_initial_speed * jump_vec;
      }
   }
}

}
