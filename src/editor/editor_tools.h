// ----------
// SNAP TOOL
// ----------

void snap_entity_to_reference(Entity* entity);
void check_selection_to_snap(EntityPanelContext* panel);
void undo_snap();
void snap_commit();


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
   auto &original = Context.original_entity_state;
   original.position  = entity->position;
   original.rotation  = entity->rotation;
   original.scale     = entity->scale;
}

void snap_entity_to_reference(Entity* entity)
{
   auto reference = Context.snap_reference;
   float diff = 0;
   auto diff_vec = vec3(0.0f);
   auto [x0, x1, z0, z1]         = reference->get_rect_bounds();
   auto [e_x0, e_x1, e_z0, e_z1] = entity->get_rect_bounds();

   switch(Context.snap_axis)
   {
      case 0:  // x
         if     (Context.snap_cycle == 0 && !Context.snap_inside) diff_vec.x = x1 - e_x0;
         else if(Context.snap_cycle == 0 &&  Context.snap_inside) diff_vec.x = x1 - e_x0 - (e_x1 - e_x0);
         else if(Context.snap_cycle == 2 && !Context.snap_inside) diff_vec.x = x0 - e_x1;
         else if(Context.snap_cycle == 2 &&  Context.snap_inside) diff_vec.x = x0 - e_x1 + (e_x1 - e_x0);
         else if(Context.snap_cycle == 1 ) diff_vec.x = x1 - e_x1 - (x1 - x0) / 2.0 + (e_x1 - e_x0) / 2.0;
         break;
      case 1:  // y
      {
         float bottom = reference->position.y;
         float height = reference->get_height();
         float top = bottom + height;
         float current_bottom = entity->position.y;
         float current_top = current_bottom + entity->get_height();

         if     (Context.snap_cycle == 0 && !Context.snap_inside) diff_vec.y = top - current_bottom;
         else if(Context.snap_cycle == 0 &&  Context.snap_inside) diff_vec.y = top - current_top;
         else if(Context.snap_cycle == 2 && !Context.snap_inside) diff_vec.y = bottom - current_top;
         else if(Context.snap_cycle == 2 &&  Context.snap_inside) diff_vec.y = bottom - current_bottom;
         else if(Context.snap_cycle == 1 && !Context.snap_inside) diff_vec.y = top - height / 2.0 - current_top;
         break;
      }
      case 2: // z
         if     (Context.snap_cycle == 0 && !Context.snap_inside) diff_vec.z = z1 - e_z0;
         else if(Context.snap_cycle == 0 &&  Context.snap_inside) diff_vec.z = z1 - e_z0 - (e_z1 - e_z0);
         else if(Context.snap_cycle == 2 && !Context.snap_inside) diff_vec.z = z0 - e_z1;
         else if(Context.snap_cycle == 2 &&  Context.snap_inside) diff_vec.z = z0 - e_z1 + (e_z1 - e_z0);
         else if(Context.snap_cycle == 1 ) diff_vec.z = z1 - e_z1 - (z1 - z0) / 2.0 + (e_z1 - e_z0) / 2.0;
         break;
   }

   entity->position += diff_vec;
}

void check_selection_to_snap(EntityPanelContext* panel)
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
   {
      Context.snap_reference = test.entity;
      snap_entity_to_reference(panel->entity);
   }
}


// -------------
// MEASURE TOOL
// -------------
void check_selection_to_measure();

void check_selection_to_measure()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
   {
      if(!Context.first_point_found || Context.second_point_found)
      {
         if(Context.second_point_found) Context.second_point_found = false;
         Context.first_point_found = true;
         Context.measure_from = point_from_detection(pickray, test);
      }
      else if(!Context.second_point_found)
      {
         Context.second_point_found = true;
         vec3 point = point_from_detection(pickray, test);
         Context.measure_to = point.y;
      }
   }
}

// -----------------
// MOVE ENTITY TOOL
// -----------------
void move_entity_with_mouse(Entity* entity);
void deselect_entity();
void check_selection_to_move_entity();
void select_entity_to_move_with_mouse(Entity* entity);


void check_selection_to_move_entity()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray, true);
   if(test.hit)
      select_entity_to_move_with_mouse(test.entity);
}

void select_entity_to_move_with_mouse(Entity* entity)
{
   Context.move_entity_with_mouse = true;
   Context.last_selected_entity = entity;
   Context.original_entity_state.position = entity->position;
   Context.original_entity_state.rotation = entity->rotation;
   Context.original_entity_state.scale    = entity->scale;
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

void undo_selected_entity_move_changes()
{
   auto entity       = Context.last_selected_entity;
   entity->position  = Context.original_entity_state.position;
   entity->scale     = Context.original_entity_state.scale;
   entity->rotate_y(Context.original_entity_state.rotation.y - entity->rotation.y);

   deselect_entity();
}

void deselect_entity()
{
   Context.move_entity_with_mouse = false;
}

// -----
// MISC
// -----
void check_for_asset_changes();
void render_aabb_boundaries(Entity* entity);

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

void render_aabb_boundaries(Entity* entity)
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

