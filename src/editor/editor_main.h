#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Editor
{

const static float TRIAXIS_SCREENPOS_X = -1.75;
const static float TRIAXIS_SCREENPOS_Y = -1.75;

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

   // toolbar
   bool toolbar_active = true;
   // measure y
   bool measure_mode = false;
   vec3 measure_from;
   bool first_point_found = false;
   bool second_point_found = false;
   float measure_to;

   // snap mode
   bool snap_mode = false;
   u8 snap_cycle = 0;
   u8 snap_axis = 1;
   bool snap_inside = true;
   Entity* snap_reference = nullptr;
   EntityState entity_state_before_snap;

   bool show_event_triggers = false;
   bool show_world_cells = false;

   Entity* tri_axis[3];
   Entity* tri_axis_letters[3];
} Context;

void initialize();
void start_frame();
void update();
void update_editor_entities();
void render();
void render_text_overlay(Player* player);
void render_toolbar();
void render_event_triggers(Camera* camera);
void render_world_cells(Camera* camera);
void end_frame();
void terminate();

#include <editor/editor_tools.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_input.h>


void update()
{
   // check for asset changes
   check_for_asset_changes();

   update_editor_entities();

   if(Context.entity_panel.active)
   {
      update_entity_control_arrows(&Context.entity_panel);
   }
   else
   {
      Context.entity_panel.rename_buffer[0] = 0;
      Context.snap_mode = false;
      Context.snap_reference = nullptr;
   }

   if(!Context.measure_mode)
   {
      Context.first_point_found  = false;
      Context.second_point_found = false;
   }

   // respond to mouse if necessary
   if(Context.move_entity_with_mouse)
   {
      if(Context.mouse_click) deselect_entity();
      else                    move_entity_with_mouse(Context.last_selected_entity);
   }
   Context.mouse_click = false;
}

void update_editor_entities()
{
   for(int i=0; i < 3; i++)
   {
	   auto entity = Context.tri_axis[i];
      glm::mat4 model = mat4identity;
		model = glm::rotate(model, glm::radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, entity->scale);
		entity->matModel = model;

      // entity = Context.tri_axis_letters[i];
      // model = mat4identity;
		// model = glm::rotate(model, glm::radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		// model = glm::rotate(model, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		// model = glm::rotate(model, glm::radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		// model = glm::scale(model, entity->scale);
		// entity->matModel = model;
   }
}

void render(Player* player)
{
   // render triaxis
   auto triaxis_view = glm::lookAt(vec3(0.0f), G_SCENE_INFO.camera->Front, -1.0f * G_SCENE_INFO.camera->Up);
   float displacement_x[3] = {0.3f, 0.0f, 0.0f};
   float displacement_y[3] = {0.0f, 0.3f, 0.0f};
   for(int i=0; i < 3; i++)
   {
      // ref. axis
	   auto entity = Context.tri_axis[i];
      entity->shader->use();
      entity->shader->setMatrix4("model", entity->matModel);
      entity->shader->setMatrix4("view", triaxis_view);
      entity->shader->setFloat2("screenPos", TRIAXIS_SCREENPOS_X, TRIAXIS_SCREENPOS_Y);
      render_entity(entity);
      // axis letter
	   // entity = Context.tri_axis_letters[i];
      // entity->shader->use();
      // entity->shader->setMatrix4("model", entity->matModel);
      // entity->shader->setMatrix4("view", triaxis_view);
      // entity->shader->setFloat2("screenPos", TRIAXIS_SCREENPOS_X + displacement_x[i], TRIAXIS_SCREENPOS_Y + displacement_y[i]);
      // render_entity(entity);
   }

   if(Context.show_event_triggers)
   {
      render_event_triggers(G_SCENE_INFO.camera);
   }

   if(Context.show_world_cells)
   {
      render_world_cells(G_SCENE_INFO.camera);
   }


   // render entity panel
   if(Context.entity_panel.active)
   {
      render_entity_panel(&Context.entity_panel);
      render_entity_control_arrows(&Context.entity_panel);
   }

   if(Context.measure_mode && Context.first_point_found && Context.second_point_found)
   {
      auto render_opts = RenderOptions();
      render_opts.always_on_top = true;
      render_opts.line_width = 2.0;

      G_IMMEDIATE_DRAW.add(
         vector<Vertex>{
            Vertex{Context.measure_from},
            Vertex{vec3(Context.measure_from.x, Context.measure_to, Context.measure_from.z)}
         },
         GL_LINE_LOOP,
         render_opts
      );
   }

   render_toolbar();

   render_text_overlay(player);

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void render_toolbar()
{
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 300, 300), ImGuiCond_Appearing);
   ImGui::Begin("Tools", &Context.toolbar_active, ImGuiWindowFlags_AlwaysAutoResize);

   if(ImGui::Button("Measure Y", ImVec2(120,18)))
   {
      Context.measure_mode = true;
   }
   if(ImGui::Button("Measure X", ImVec2(120,18)))
   {
   }
   if(ImGui::Button("Measure Z", ImVec2(120,18)))
   {
   }
   
   ImGui::NewLine();
   ImGui::Checkbox("Show Event Triggers", &Context.show_event_triggers);
   ImGui::Checkbox("Show World Cells", &Context.show_world_cells);

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

   Context.tri_axis[0] = x_axis;
   Context.tri_axis[1] = y_axis;
   Context.tri_axis[2] = z_axis;

   auto letter_x_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_x");
   auto x_axis_letter = new Entity();
   x_axis_letter->mesh = letter_x_mesh;
   x_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "blue.jpg",  "blue axis"});
   x_axis_letter->shader = shader;
   x_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   Context.tri_axis_letters[0] = x_axis_letter;

   auto letter_y_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_y");
   auto y_axis_letter = new Entity();
   y_axis_letter->mesh = letter_y_mesh;
   y_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "green.jpg",  "green axis"});
   y_axis_letter->shader = shader;
   y_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   Context.tri_axis_letters[1] = y_axis_letter;

   auto letter_z_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_z");
   auto z_axis_letter = new Entity();
   z_axis_letter->mesh = letter_z_mesh;
   z_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "pink.jpg",  "pink axis"});
   z_axis_letter->shader = shader;
   z_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   Context.tri_axis_letters[2] = z_axis_letter;

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
   if(Context.snap_mode)
   {
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

      vec3 snap_mode_color;
      if(Context.entity_state_before_snap.position != Context.entity_panel.entity->position)
         snap_mode_color = vec3(0.8, 0.8, 0.2);
      else
         snap_mode_color = vec3(0.6, 1.0, 0.3);


      render_text("SNAP MODE ON (" + snap_axis + "-" + snap_cycle + ")",
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, GUI_y - 60, 3.0, snap_mode_color, true
      );
   }

   if(Context.measure_mode)
   {
      render_text("MEASURE MODE ON",
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, GUI_y - 60, 3.0, vec3(0.8, 0.8, 0.2), true
      );
      if(Context.second_point_found)
      {
         render_text("(" + format_float_tostr(abs(Context.measure_to - Context.measure_from.y), 2) + " m)",
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, GUI_y - 80, 2.0, vec3(0.8, 0.8, 0.2), true
         ); 
      }
   }
}

void render_event_triggers(Camera* camera)
{
   auto checkpoints = G_SCENE_INFO.active_scene->checkpoints;
   if(checkpoints.size() == 0)
      return;
      
   auto checkpoint = checkpoints[0];
   // RenderOptions render_opts;
   // render_opts.wireframe = true;

   auto find = Shader_Catalogue.find("color");
   auto shader = find->second;

   for(int i = 0; i < checkpoints.size(); i++)
   {
      shader->use();
      shader->setFloat3("color", 0.5, 0.5, 0.3);
      shader->setFloat("opacity", 0.6);
      shader->setMatrix4("model", checkpoint->trigger_model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(checkpoint->trigger, RenderOptions{});
      checkpoint++;
   }
}

void render_world_cells(Camera* camera)
{
   auto& scene = G_SCENE_INFO.active_scene;

   // get unique world cells references that are currently in use; 
   vector<WorldCell*> cells;
   Entity **entity_iterator = &(scene->entities[0]);
   int entities_vec_size =  scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      for(int c = 0; c < entity->world_cells_count; c++)
      {
         auto entity_wc = entity->world_cells[c];
         bool exists = false;
         for(int wc = 0; wc < cells.size(); wc++)
         {
            if(cells[wc] == entity_wc)
            {
               exists = true;
               break;
            }
         }
         if(exists) continue;
         
         cells.push_back(entity_wc);
      }
   }

   // render
   auto shader = Shader_Catalogue.find("color")->second;
   auto cell_mesh = Geometry_Catalogue.find("world cell")->second;
   RenderOptions opts;
   opts.wireframe = true;
   for(int i = 0; i < cells.size(); i++)
   {
      // create model matrix
      vec3 position = get_world_coordinates_from_world_cell_coordinates(
         cells[i]->i, cells[i]->j, cells[i]->k
      );
      glm::mat4 model = translate(mat4identity, position);
		model = glm::scale(model, vec3{WORLD_CELL_SIZE, WORLD_CELL_SIZE, WORLD_CELL_SIZE});

      shader->use();
      shader->setFloat3("color", 0.27, 0.55, 0.65);
      shader->setFloat("opacity", 0.85);
      shader->setMatrix4("model", model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(cell_mesh, opts);
   }

}

}
