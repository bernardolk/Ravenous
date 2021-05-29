
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>


namespace Editor
{

struct EntityPanelContext {
   bool active = false;
   Entity* entity = nullptr;
   vec3 original_position = vec3(0);
   vec3 original_scale = vec3(0);
   float original_rotation = 0;
   char rename_buffer[100];
   bool reverse_scale_x = false;
   bool reverse_scale_y = false;
   bool reverse_scale_z = false;
   Entity* x_arrow;
   Entity* y_arrow;
   Entity* z_arrow;
};

struct EntityState {
   vec3 position;
   vec3 scale;
   vec3 rotation;
};

struct EditorContext {
   ImGuiStyle* imStyle;
   EntityPanelContext entity_panel;

   bool move_entity_with_mouse = false;
   bool mouse_click = false;
   Entity* last_selected_entity = nullptr;
   EntityState original_entity_state;

   // snap mode
   bool snap_mode = false;
   u8 snap_cycle = 0;
   u8 snap_axis = 1;
   EntityState entity_state_before_snap;

   vector<Entity*> entities;
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
void update_editor_entities();
void check_for_asset_changes();
void update_entity_control_arrows(EntityPanelContext* panel);
void render_entity_control_arrows(EntityPanelContext* panel);
void check_selection_to_snap(EntityPanelContext* panel);
void render_text_overlay(Player* player);
void snap_entity_to_reference(Entity* entity, Entity* reference);



void update()
{
   // check for asset changes
   check_for_asset_changes();

   //debug_entities(); // ACTIVATE this for editor mode entity debugging code

   update_editor_entities();

   // respond to mouse if necessary
   if(Context.move_entity_with_mouse)
   {
      if(Context.mouse_click)
         deselect_entity();
      else
         move_entity_with_mouse(Context.last_selected_entity);
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

void check_for_asset_changes()
{
   auto it = Geometry_Catalogue.begin();
   while (it != Geometry_Catalogue.end())
   {
      auto model_name = it->first;
      string path = MODELS_PATH + model_name + ".obj";

      WIN32_FIND_DATA find_data;
      HANDLE find_handle = FindFirstFileA(path.c_str(), &find_data);
      if(find_handle != INVALID_HANDLE_VALUE)
      {
        auto mesh = it->second;
         if(CompareFileTime(&mesh->last_written, &find_data.ftLastWriteTime) != 0)
         {
            cout << "ASSET '" << model_name << "' changed!\n";
            mesh->last_written = find_data.ftLastWriteTime;
            auto dummy_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, model_name);
            mesh->vertices = dummy_mesh->vertices;
            mesh->indices = dummy_mesh->indices;
            mesh->gl_data = dummy_mesh->gl_data;
            free(dummy_mesh);
         }
      }

      FindClose(find_handle);
      it++;
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
   auto bounds = entity->collision_geometry.aabb;
   G_IMMEDIATE_DRAW.add(
      vector<Vertex>{
         Vertex{vec3(bounds.x0,entity->position.y, bounds.z0)},
         Vertex{vec3(bounds.x0,entity->position.y + bounds.height, bounds.z0)},

         Vertex{vec3(bounds.x0,entity->position.y, bounds.z1)},
         Vertex{vec3(bounds.x0,entity->position.y + bounds.height, bounds.z1)},

         Vertex{vec3(bounds.x1,entity->position.y, bounds.z1)},
         Vertex{vec3(bounds.x1,entity->position.y + bounds.height, bounds.z1)},

         Vertex{vec3(bounds.x1,entity->position.y, bounds.z0)},
         Vertex{vec3(bounds.x1,entity->position.y + bounds.height, bounds.z0)},
      },
      GL_POINTS
   );
}


void check_selection_to_open_panel()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
      set_entity_panel(test.entity);
}

void check_selection_to_move_entity()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray, true);
   if(test.hit)
      select_entity(test.entity);
}

void check_selection_to_snap(EntityPanelContext* panel)
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
   {
      snap_entity_to_reference(panel->entity, test.entity);
   }
}

void snap_entity_to_reference(Entity* entity, Entity* reference)
{
   float bottom = reference->position.y;
   float height = reference->get_height();
   float top = bottom + height;
   float current_bottom = entity->position.y;
   float current_top = current_bottom + entity->get_height();
   float diff = 0;
   vec3  diff_vec;
   switch(Context.snap_cycle)
   {
      case 0:
         diff_vec = vec3{0, top - current_top, 0};
         break;
      case 1:
         diff_vec = vec3{0, top - height / 2.0 - current_top, 0};
         break;
      case 2:
         diff_vec = vec3{0, bottom - current_top, 0};
         break;
   }

   entity->position += diff_vec;
}

void undo_snap()
{
   auto entity       = Context.entity_panel.entity;
   entity->position  = Context.entity_state_before_snap.position;
   entity->scale     = Context.entity_state_before_snap.scale;
   entity->rotate_y(Context.entity_state_before_snap.rotation.y - entity->rotation.y);
}

void snap_commit()
{
   auto entity    = Context.entity_panel.entity;
   auto &undo     = Context.entity_state_before_snap;
   undo.position  = entity->position;
   undo.rotation  = entity->rotation;
   undo.scale     = entity->scale;
}

void set_entity_panel(Entity* entity)
{
   Context.last_selected_entity = entity;

   auto &undo     = Context.original_entity_state;
   undo.position  = entity->position;
   undo.rotation  = entity->rotation;
   undo.scale     = entity->scale;

   // could be made using EntityState? could, but I am lazy.
   auto &panel = Context.entity_panel;
   panel.active = true;
   panel.entity = entity;
   panel.original_position = entity->position;
   panel.original_scale    = entity->scale;
   panel.original_rotation = entity->rotation.y;
   panel.reverse_scale_x = false;
   panel.reverse_scale_y = false;
   panel.reverse_scale_z = false;
   panel.x_arrow->rotation = vec3{0,0,270};
   panel.y_arrow->rotation = vec3{0,0,0};
   panel.z_arrow->rotation = vec3{90,0,0};
}




void update_editor_entities()
{
	Entity **entity_iterator = &(Context.entities[0]);
   for(int it=0; it < Context.entities.size(); it++)
   {
	   auto &entity = *entity_iterator++;

      // auto x_rads = acos(G_SCENE_INFO.camera->Front.x);
      // auto x_rads_s = asin(G_SCENE_INFO.camera->Front.x);
      // auto x_angle_c = glm::degrees(x_rads);
      // auto x_angle_s = glm::degrees(x_rads_s);
      // entity->rotation.x = x_angle_c;
      // entity->rotation.z = x_angle_s;

      // its something like that
      // auto angle_xy = glm::degrees(atan(G_SCENE_INFO.camera->Front.y / G_SCENE_INFO.camera->Front.x));
      // entity->rotation.z = G_SCENE_INFO.camera->Yaw;
      // entity->rotation.x = G_SCENE_INFO.camera->Pitch;
      

      glm::mat4 model = translate(mat4identity, entity->position);
		model = glm::rotate(model, glm::radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, entity->scale);
		entity->matModel = model;
   }
}

void render(Player* player)
{
   // render editor entities
	Entity **entity_iterator = &(Context.entities[0]);
   for(int it=0; it < Context.entities.size(); it++)
   {
	   auto entity = *entity_iterator++;
      entity->shader->use();
      // important that the gizmo dont have a position set.
      entity->shader->setMatrix4("model", entity->matModel);
      // extract rotation out of the camera view mat4 and set into gizmo
      vec3 scale, trans, skew;
      glm::vec4 perspective;
      glm::quat orientation;
	   auto test = glm::lookAt(vec3(0.0f), G_SCENE_INFO.camera->Front, -1.0f * G_SCENE_INFO.camera->Up);
      entity->shader-> setMatrix4("view", test);

      render_entity(entity);
   }

   // render entity panel
   if(Context.entity_panel.active)
   {
      render_entity_panel(&Context.entity_panel);
      if(Context.entity_panel.entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
      {
         auto panel = &Context.entity_panel;
         update_entity_control_arrows(panel);
         render_entity_control_arrows(panel);
      }
   }
   else
   {
      Context.entity_panel.rename_buffer[0] = 0;
      Context.snap_mode = false;
   } 

   render_text_overlay(player);

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void select_entity(Entity* entity)
{
   Context.move_entity_with_mouse = true;
   Context.last_selected_entity = entity;
   Context.original_entity_state.position = entity->position;
   Context.original_entity_state.rotation = entity->rotation;
   Context.original_entity_state.scale    = entity->scale;
}

void deselect_entity()
{
   Context.move_entity_with_mouse = false;
}


void undo_entity_panel_changes()
{
   auto entity       = Context.entity_panel.entity;
   entity->position  = Context.entity_panel.original_position;
   entity->scale     = Context.entity_panel.original_scale;
   entity->rotate_y(Context.entity_panel.original_rotation - entity->rotation.y);
}

void undo_selected_entity_move_changes()
{
   auto entity       = Context.last_selected_entity;
   entity->position  = Context.original_entity_state.position;
   entity->scale     = Context.original_entity_state.scale;
   entity->rotate_y(Context.original_entity_state.rotation.y - entity->rotation.y);

   deselect_entity();
}


void render_entity_panel(EntityPanelContext* panel_context)
{
   auto& entity = panel_context->entity;
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("Entity Panel", &panel_context->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text(entity->name.c_str());
   
   //rename
   ImGui::NewLine();
   if(ImGui::InputText("rename", &panel_context->rename_buffer[0], 100))
      entity->name = panel_context->rename_buffer;

   ImGui::NewLine();
   // position
   {
      ImGui::SliderFloat(
         "x",
         &entity->position.x,
         panel_context->original_position.x - 4,
         panel_context->original_position.x + 4
      );
      ImGui::SliderFloat(
         "y",
         &entity->position.y,
         panel_context->original_position.y - 4,
         panel_context->original_position.y + 4
      );
      ImGui::SliderFloat(
         "z", 
         &entity->position.z, 
         panel_context->original_position.z - 4, 
         panel_context->original_position.z + 4
      );
   }

   // rotation
   {
      float rotation = entity->rotation.y;
      if(ImGui::InputFloat("rot y", &rotation, 90))
         Context.entity_panel.entity->rotate_y(rotation - entity->rotation.y);
   }

   ImGui::NewLine();

   // scale
   {
      auto scale = entity->scale;

      auto rot = glm::rotate(mat4identity, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
      scale = rot * vec4(entity->scale, 1.0f);
      auto ref_scale = scale;

      bool flipped_x = false, flipped_z = false;
      if(scale.x < 0) { scale.x *= -1; flipped_x = true;}
      if(scale.z < 0) { scale.z *= -1; flipped_z = true;}
      
      vec3 min_scales {0.0f};

      // scale in x
      bool scaled_x = ImGui::SliderFloat(
         "scale x",
         &scale.x,
         min_scales.x,
         panel_context->original_scale.x + 4
      );
      if(ImGui::Checkbox("rev x", &panel_context->reverse_scale_x))
         panel_context->x_arrow->rotation.z = (int)(panel_context->x_arrow->rotation.z + 180) % 360;

      // scale in y
      bool scaled_y = ImGui::SliderFloat(
         "scale y",
         &scale.y,
         min_scales.y,
         panel_context->original_scale.y + 4
      );
      if(ImGui::Checkbox("rev y", &panel_context->reverse_scale_y))
         panel_context->y_arrow->rotation.z = (int)(panel_context->y_arrow->rotation.z + 180) % 360;

      // scale in z
      bool scaled_z = ImGui::SliderFloat(
         "scale z", 
         &scale.z,
         min_scales.z,
         panel_context->original_scale.z + 4
      );
      if(ImGui::Checkbox("rev z", &panel_context->reverse_scale_z))
         panel_context->z_arrow->rotation.x = (int)(panel_context->z_arrow->rotation.x + 180) % 360;

      // apply scalling
      if(scaled_x || scaled_y || scaled_z)
      {
         if(flipped_x) scale.x *= -1; 
         if(flipped_z) scale.z *= -1; 

          // if rev scaled, move entity in oposite direction to compensate scaling and fake rev scaling
         if((panel_context->reverse_scale_x && !flipped_x) || (flipped_x && !panel_context->reverse_scale_x))
            entity->position.x -= scale.x - ref_scale.x;
         if(panel_context->reverse_scale_y)
            entity->position.y -= scale.y - entity->scale.y;
         if((panel_context->reverse_scale_z && !flipped_z) || (flipped_z && !panel_context->reverse_scale_z))
            entity->position.z -= scale.z - ref_scale.z;

         auto inv_rot = glm::rotate(mat4identity, glm::radians(-1.0f * entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
         scale = inv_rot * vec4(scale, 1.0f);

         entity->scale = scale;
      }
   }
   
   ImGui::NewLine();

   // Controls
   {
      if(ImGui::Button("Snap", ImVec2(82,18)))
      {
         Context.snap_mode = true;
         auto &undo_snap     = Context.entity_state_before_snap;
         undo_snap.position  = entity->position;
         undo_snap.rotation  = entity->rotation;
         undo_snap.scale     = entity->scale;
      }
      if(ImGui::Button("Duplicate", ImVec2(82,18)))
      {
         auto new_entity = copy_entity(entity);
         new_entity->name += " copy";
         bool already_exists = get_entity_position(G_SCENE_INFO.active_scene, new_entity) > -1;
         if(already_exists) new_entity->name += "(1)";
         G_SCENE_INFO.active_scene->entities.push_back(new_entity);
         select_entity(new_entity);
         set_entity_panel(new_entity);
      }

      if(ImGui::Button("Erase", ImVec2(82,18)))
      {
         auto& list = G_SCENE_INFO.active_scene->entities;
         int index = get_entity_position(G_SCENE_INFO.active_scene, entity);
         list.erase(list.begin() + index);
         Context.entity_panel.active = false;
      }

      ImGui::Checkbox("Hide", &entity->wireframe);
   }

   ImGui::End();
}

void update_entity_control_arrows(EntityPanelContext* panel)
{
   auto entity = panel->entity;

   if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
      return;

   auto &x = panel->x_arrow;
   auto &y = panel->y_arrow;
   auto &z = panel->z_arrow;

   auto collision = entity->collision_geometry;

   x->position = entity->position;
   y->position = entity->position;
   z->position = entity->position;

   // change scale from local to world coordinates
   auto rot = glm::rotate(mat4identity, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   vec3 scale_w_coords = rot * vec4(entity->scale, 1.0f);

   if(!panel->reverse_scale_x) x->position.x = max(x->position.x, x->position.x + scale_w_coords.x);
   else                        x->position.x = min(x->position.x, x->position.x + scale_w_coords.x);
   x->position.y += scale_w_coords.y / 2.0f;
   x->position.z += scale_w_coords.z / 2.0f;

   if(!panel->reverse_scale_y) y->position.y = max(y->position.y, y->position.y + scale_w_coords.y);
   else                        y->position.y = min(y->position.y, y->position.y + scale_w_coords.y);
   y->position.x += scale_w_coords.x / 2.0f;
   y->position.z += scale_w_coords.z / 2.0f;

   if(!panel->reverse_scale_z) z->position.z = max(z->position.z, z->position.z + scale_w_coords.z);
   else                        z->position.z = min(z->position.z, z->position.z + scale_w_coords.z);
   z->position.y += scale_w_coords.y / 2.0f;
   z->position.x += scale_w_coords.x / 2.0f;

   x->update_model_matrix();
   y->update_model_matrix();
   z->update_model_matrix();
}

//@todo: needs refactoring
void render_entity_control_arrows(EntityPanelContext* panel)
{
   glDepthFunc(GL_ALWAYS); 
   render_editor_entity(panel->x_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->y_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->z_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   glDepthFunc(GL_LESS); 
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

   // load tri axis gizmo
   auto axis_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "axis");

   auto x_axis = new Entity();
   auto y_axis = new Entity();
   auto z_axis = new Entity();

   x_axis->mesh = axis_mesh;
   y_axis->mesh = axis_mesh;
   z_axis->mesh = axis_mesh;

   auto blue_tex  = load_texture_from_file("blue.jpg",   TEXTURES_PATH);
   auto green_tex = load_texture_from_file("green.jpg",  TEXTURES_PATH);
   auto pink_tex  = load_texture_from_file("pink.jpg",   TEXTURES_PATH);

   x_axis->textures.push_back(Texture{blue_tex,  "texture_diffuse", "blue.jpg",  "blue axis"});
   y_axis->textures.push_back(Texture{green_tex, "texture_diffuse", "green.jpg", "green axis"});
   z_axis->textures.push_back(Texture{pink_tex,  "texture_diffuse", "pink.jpg",  "pink axis"});

   auto shader = Shader_Catalogue.find("static")->second;
   x_axis->shader = shader;
   x_axis->scale = vec3{0.1, 0.1, 0.1};
   x_axis->rotation = vec3{90, 0, 90};

   y_axis->shader = shader;
   y_axis->scale = vec3{0.1, 0.1, 0.1};
   y_axis->rotation = vec3{180, 0, 0};

   z_axis->shader = shader;
   z_axis->scale = vec3{0.1, 0.1, 0.1};
   z_axis->rotation = vec3{90, 0, 180};

   Context.entities.push_back(x_axis);
   Context.entities.push_back(y_axis);
   Context.entities.push_back(z_axis);

   // load entity panel axis arrows
   auto x_arrow = new Entity();
   auto y_arrow = new Entity();
   auto z_arrow = new Entity();

   x_arrow->mesh = axis_mesh;
   y_arrow->mesh = axis_mesh;
   z_arrow->mesh = axis_mesh;

   auto model_shader = Shader_Catalogue.find("model")->second;
   x_arrow->shader = model_shader;
   x_arrow->scale = vec3{0.5,0.5,0.5};
   x_arrow->rotation = vec3{0,0,270};

   y_arrow->shader = model_shader;
   y_arrow->scale = vec3{0.5,0.5,0.5};
   y_arrow->rotation = vec3{0,0,0};

   z_arrow->shader = model_shader;
   z_arrow->scale = vec3{0.5,0.5,0.5};
   z_arrow->rotation = vec3{90,0,0};

   x_arrow->textures.push_back(Texture{blue_tex,  "texture_diffuse", "blue.jpg",  "blue axis"});
   y_arrow->textures.push_back(Texture{green_tex, "texture_diffuse", "green.jpg", "green axis"});
   z_arrow->textures.push_back(Texture{pink_tex,  "texture_diffuse", "pink.jpg",  "pink axis"});

   Context.entity_panel.x_arrow = x_arrow;
   Context.entity_panel.y_arrow = y_arrow;
   Context.entity_panel.z_arrow = z_arrow;
}

void handle_input_flags(InputFlags flags, Player* &player)
{
   if(Context.snap_mode == true)
   {
      if(pressed_once(flags, KEY_ESC))
      {
         Context.snap_mode = false;
      }
      else if(pressed_once(flags, KEY_ENTER))
      {
         snap_commit();
      }
      else if(pressed_only(flags, KEY_X))
      {
         if(Context.snap_axis == 0)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            undo_snap();
            Context.snap_cycle = 0;
            Context.snap_axis = 0;
         }
      }
      else if(pressed_only(flags, KEY_Y))
      {
         if(Context.snap_axis == 1)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            undo_snap();
            Context.snap_cycle = 0;
            Context.snap_axis = 1;
         }
      }
      else if(pressed_only(flags, KEY_Z))
      {
         if(Context.snap_axis == 2)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            undo_snap();
            Context.snap_cycle = 0;
            Context.snap_axis = 2;
         }
      }
   }
   
   if(flags.key_press & KEY_LEFT_CTRL && pressed_once(flags, KEY_Z))
   {
      if(Context.entity_panel.active && Context.snap_mode == false)
         undo_entity_panel_changes();
      else if (Context.snap_mode == true)
         undo_snap();
      else if(Context.last_selected_entity != nullptr)
         undo_selected_entity_move_changes();
   }

   if(ImGui::GetIO().WantCaptureKeyboard)
      return;

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
      if(Context.snap_mode)
         check_selection_to_snap(&Context.entity_panel);
      else
         check_selection_to_open_panel();
   }
   else if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK && flags.key_press & KEY_G)
   {
      check_selection_to_move_entity();
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
      player->height_before_fall = player->entity_ptr->position.y;
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

void render_text_overlay(Player* player) 
{
   auto camera = G_SCENE_INFO.camera;
   string player_floor = "player floor: ";
   if(player->standing_entity_ptr != NULL)
      player_floor += player->standing_entity_ptr->name;

   string lives = to_string(player->lives);

   string GUI_atts[]{
      format_float_tostr(camera->Position.x, 2),               //0
      format_float_tostr(camera->Position.y,2),                //1
      format_float_tostr(camera->Position.z,2),                //2
      format_float_tostr(camera->Pitch,2),                     //3
      format_float_tostr(camera->Yaw,2),                       //4
      format_float_tostr(camera->Front.x,2),                   //5
      format_float_tostr(camera->Front.y,2),                   //6
      format_float_tostr(camera->Front.z,2),                   //7
      format_float_tostr(player->entity_ptr->position.x,1),    //8
      format_float_tostr(player->entity_ptr->position.y,1),    //9 
      format_float_tostr(player->entity_ptr->position.z,1),    //10
      format_float_tostr(G_FRAME_INFO.time_step,1)             //11
   };

   string cam_type = camera->type == FREE_ROAM ? "FREE ROAM" : "THIRD PERSON";
   string camera_type_string  = "camera type: " + cam_type;
   string camera_position  = "camera:   x: " + GUI_atts[0] + " y:" + GUI_atts[1] + " z:" + GUI_atts[2];
   string camera_front     = "    dir:  x: " + GUI_atts[5] + " y:" + GUI_atts[6] + " z:" + GUI_atts[7];
   string mouse_stats      = "    pitch: " + GUI_atts[3] + " yaw: " + GUI_atts[4];
   string fps              = to_string(G_FRAME_INFO.current_fps);
   string fps_gui          = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);
   string player_pos       = "player:   x: " +  GUI_atts[8] + " y: " +  GUI_atts[9] + " z: " +  GUI_atts[10];
   string time_step_string = "time step: " + GUI_atts[11] + "x";


   vec3 player_state_text_color;
   std::string player_state_text;
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
         player_state_text_color = vec3(0, 0.8, 0.1);
         player_state_text = "PLAYER STANDING";
         break;
      case PLAYER_STATE_FALLING:
         player_state_text_color = vec3(0.8, 0.1, 0.1);
         player_state_text = "PLAYER FALLING";
         break;
      case PLAYER_STATE_FALLING_FROM_EDGE:
         player_state_text_color = vec3(0.8, 0.1, 0.3);
         player_state_text = "PLAYER FALLING FROM EDGE";
         break;
      case PLAYER_STATE_JUMPING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER JUMPING";
         break;
      case PLAYER_STATE_SLIDING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER SLIDING";
         break;
      case PLAYER_STATE_SLIDE_FALLING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER SLIDE FALLING";
         break;
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER EVICTED FROM SLOPE";
         break;
   }

   std::string view_mode_text;
   switch(PROGRAM_MODE.current)
   {
      case EDITOR_MODE:
         view_mode_text = "EDITOR MODE";
         break;
      case GAME_MODE:
         view_mode_text = "GAME MODE";
         break;
   }

   float GUI_x = 25;
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;

   render_text(camera_type_string,  GUI_x, GUI_y,        1.3);
   render_text(camera_position,     GUI_x, GUI_y - 30,   1.3);
   render_text(player_pos,          GUI_x, GUI_y - 60,   1.3);

   render_text(lives,               G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 90, 1.3,  
      player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1}
   );
   render_text(player_floor,        G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 60, 1.3);
   render_text(player_state_text,   G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 30, 1.3, player_state_text_color);

   render_text(view_mode_text,            G_DISPLAY_INFO.VIEWPORT_WIDTH - 200, GUI_y,        1.3);
   render_text(G_SCENE_INFO.scene_name,   G_DISPLAY_INFO.VIEWPORT_WIDTH - 200, GUI_y - 30,   1.3, vec3(0.8, 0.8, 0.2));
   render_text(time_step_string,          G_DISPLAY_INFO.VIEWPORT_WIDTH - 200, GUI_y - 60,   1.3, vec3(0.8, 0.8, 0.2));
   render_text(fps_gui,                   G_DISPLAY_INFO.VIEWPORT_WIDTH - 200, GUI_y - 90,   1.3);

   // render snap mode indicator
   string snap_cycle;
   switch(Context.snap_cycle)
   {
      case 0:
         snap_cycle = "top";
         break;
      case 1:
         snap_cycle = "mid";
         break;
      case 2:
         snap_cycle = "bottom";
         break;
   }

   string snap_axis;
   switch(Context.snap_axis)
   {
      case 0:
         snap_axis = "X";
         break;
      case 1:
         snap_axis = "Y";
         break;
      case 2:
         snap_axis = "Z";
         break;
   }

   if(Context.snap_mode)
      render_text("SNAP MODE ON (" + snap_axis + "-" + snap_cycle + ")", 
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, GUI_y - 60, 3.0, vec3(0.8, 0.8, 0.2), true);
}

}
