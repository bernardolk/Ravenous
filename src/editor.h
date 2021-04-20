

namespace Editor
{

struct EntityPanelContext {
   Entity* entity = NULL;
   vec3 original_position = vec3(0);
   bool active = false;
};

struct EditorContext {
   ImGuiStyle* imStyle;
   EntityPanelContext entity_panel;
} Context;

void editor_check_clicking();
void render();
void select_entity(Entity* entity);
void render_entity_panel(EntityPanelContext* context);
void start_frame();
void end_frame();
void initialize();
void terminate();



void editor_check_clicking()
{
   if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK)
   {
      auto pickray = cast_pickray();
      auto test = test_ray_against_scene(pickray);
      if(test.hit)
      {
         cout << "HIT ENTITY '" + test.entity->name + "' :D \n";
      }
   }
}

void render()
{
   if(Context.entity_panel.active)
      render_entity_panel(&Context.entity_panel);
}


void select_entity(Entity* entity)
{
   Context.entity_panel.entity = entity;
   Context.entity_panel.active = true;
   Context.entity_panel.original_position = vec3{
      entity->position
   };
}

void render_entity_panel(EntityPanelContext* context)
{
   auto entity = context->entity;
   ImGui::Begin(entity->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
   ImGui::SliderFloat(
      "x", 
      &context->entity->position.x, 
      context->original_position.x - 50, 
      context->entity->position.x + 50
   );
   ImGui::SliderFloat(
      "y",
      &context->entity->position.y,
      context->original_position.y - 50,
      context->entity->position.x + 50
   );
   ImGui::SliderFloat(
      "z", 
      &context->entity->position.z, 
      context->original_position.z - 50, 
      context->entity->position.x + 50
   );

   ImGui::SliderFloat("rot x", &context->entity->rotation.x, -360, 360);
   ImGui::SliderFloat("rot y", &context->entity->rotation.y, -360, 360);
   ImGui::SliderFloat("rot z", &context->entity->rotation.z, -360, 360);
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
   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
