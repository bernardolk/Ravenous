
namespace Editor
{

struct EntityPanelContext {
   Entity* entity = NULL;
   vec3 original_position = vec3(0);
   vec3 original_scale = vec3(0);
   bool active = false;
};

struct EditorContext {
   ImGuiStyle* imStyle;
   EntityPanelContext entity_panel;
} Context;

void check_selection_click();
void render();
void select_entity(Entity* entity);
void render_entity_panel(EntityPanelContext* context);
void start_frame();
void end_frame();
void initialize();
void terminate();

void debug_entities()
{
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;

      if(entity->name == "small upper platform")
      {
         //auto temp_col = entity->collision_geometry.aabb;
         G_IMMEDIATE_DRAW.add(1, &entity->position);
         //G_IMMEDIATE_DRAW.add(entity.position + vec3(temp_col.length_x, 0, 0));
         //G_IMMEDIATE_DRAW.add(entity.position);
      }
   }
}


void check_selection_click()
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

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void select_entity(Entity* entity)
{
   Context.entity_panel.entity = entity;
   Context.entity_panel.active = true;
   Context.entity_panel.original_position = vec3{
      entity->position
   };
   Context.entity_panel.original_scale = vec3{
      entity->scale
   };
}

void render_entity_panel(EntityPanelContext* context)
{
   auto entity = context->entity;
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin(entity->name.c_str(), &context->active, ImGuiWindowFlags_AlwaysAutoResize);

   // position
   ImGui::SliderFloat(
      "x",
      &context->entity->position.x,
      context->original_position.x - 4,
      context->original_position.x + 4
   );
   ImGui::SliderFloat(
      "y",
      &context->entity->position.y,
      context->original_position.y - 4,
      context->original_position.y + 4
   );
   ImGui::SliderFloat(
      "z", 
      &context->entity->position.z, 
      context->original_position.z - 4, 
      context->original_position.z + 4
   );

   // rotation
   float rotation = context->entity->rotation.y;
   if(ImGui::InputFloat("rot y", &rotation, 90))
      Context.entity_panel.entity->rotate_y(rotation - context->entity->rotation.y);

   // scale
   auto scale = vec3{context->entity->scale};
   if(ImGui::SliderFloat(
      "scale x",
      &scale.x,
      context->original_scale.x - 4,
      context->original_scale.x + 4
   ) ||
   ImGui::SliderFloat(
      "scale y",
      &scale.y,
      context->original_scale.y - 4,
      context->original_scale.y + 4
   ) ||
   ImGui::SliderFloat(
      "scale z", 
      &scale.z,
      context->original_scale.z - 4,
      context->original_scale.z + 4
   ))
   {
      Context.entity_panel.entity->set_scale(scale);
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

}
