#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Editor
{
const static string EDITOR_ASSETS = PROJECT_PATH + "/assets/editor/";

const static float TRIAXIS_SCREENPOS_X = -1.80;
const static float TRIAXIS_SCREENPOS_Y = -1.80;

#include <editor/editor_undo.h>

struct PalettePanelContext {
   bool active = true;
   unsigned int textures[15];
   EntityAttributes entity_palette[15];
   unsigned int count = 0;
};

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
   bool action_last_frame = false;
   EntityState entity_tracked_state;
};

struct WorldPanelContext {
   bool active = false;
   vec3 cell_coords = vec3{-1.0f};
};

struct LightsPanelContext {
   bool active = false;
   bool focus_tab = false;

   // selected light
   int selected_light = -1;
   float selected_light_yaw;
   float selected_light_pitch;
   string selected_light_type;
};

struct EditorContext {
   ImGuiStyle* imStyle;
   
   // scene tracking
   string last_frame_scene;

   // undo stack
   UndoStack undo_stack;

   // deletion log
   DeletedEntityLog deletion_log;

   // panels
   EntityPanelContext entity_panel;
   WorldPanelContext world_panel;
   PalettePanelContext palette_panel;
   LightsPanelContext lights_panel;

   // toolbar
   bool toolbar_active = true;

   // general mode controls
   bool mouse_click = false;
   Entity* selected_entity = nullptr;

   // move mode
   bool move_mode = false;
   bool scale_on_drop = false;
   u8 move_axis = 0;

   // move light @todo: will disappear!
   string selected_light_type = "";
   int selected_light = -1;

   // scale mode
   bool scale_entity_with_mouse = false;
   
   // measure mode
   bool measure_mode = false;
   vec3 measure_from;
   bool first_point_found = false;
   bool second_point_found = false;
   float measure_to;

   // locate coordinates mode
   bool locate_coords_mode = false;
   bool locate_coords_found_point = false;
   vec3 locate_coords_position;

   // snap mode
   bool snap_mode = false;
   u8 snap_cycle = 0;
   u8 snap_axis = 1;
   bool snap_inside = true;
   Entity* snap_reference = nullptr;
   EntityState snap_tracked_state;

   // show things 
   bool show_event_triggers = false;
   bool show_world_cells = false;
   bool show_lightbulbs = true;

   // gizmos
   Entity* tri_axis[3];
   Entity* tri_axis_letters[3];
} Context;


void initialize();
void start_frame();
void update();
void update_editor_entities();
void check_selection_to_open_panel();
void check_selection_to_move_entity();
void render(Player* player, WorldStruct* world);
void render_text_overlay(Player* player);
void render_toolbar();
void render_event_triggers(Camera* camera);
void render_world_cells(Camera* camera);
void render_lightbulbs(Camera* camera);
void end_frame();
void terminate();


#include <editor/editor_tools.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_world_panel.h>
#include <editor/editor_input.h>
#include <editor/editor_palette_panel.h>
#include <editor/editor_lights_panel.h>


void update()
{
   if(Context.last_frame_scene != G_SCENE_INFO.scene_name)
   {
      Context.entity_panel.active = false;
      Context.world_panel.active = false;
   }

   Context.last_frame_scene = G_SCENE_INFO.scene_name;

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

   // unselect lights when not panel is not active
   if(!Context.lights_panel.active)
   {
      Context.lights_panel.selected_light = -1;
      Context.lights_panel.selected_light_type = "";
   }
   else if(
      Context.lights_panel.selected_light != -1 &&
      Context.lights_panel.selected_light_type != ""
   )
   {
      Context.show_lightbulbs = true;
   }


   // set editor mode values to initial if not active
   if(!Context.measure_mode)
   {
      Context.first_point_found  = false;
      Context.second_point_found = false;
   }
   if(!Context.snap_mode)
   {
      Context.snap_cycle = 0;
      Context.snap_axis = 1;
      Context.snap_inside = true;
      Context.snap_reference = nullptr;
   }

   // respond to mouse if necessary
   if(Context.move_mode)
   {
      if(Context.mouse_click)
      {
         if(Context.selected_light > -1)
            place_light();
         else
            place_entity();
      }
      else
      {
         if(Context.selected_light > -1)
            move_light_with_mouse(Context.selected_light_type, Context.selected_light);
         else
            move_entity_with_mouse(Context.selected_entity);
      }
   }

   if(Context.scale_entity_with_mouse)
   {
      scale_entity_with_mouse(Context.selected_entity);
   }

   // resets mouse click event
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
   }
}

void render(Player* player, WorldStruct* world)
{
   // render world objs if toggled
   if(Context.show_event_triggers)
   {
      render_event_triggers(G_SCENE_INFO.camera);
   }

   if(Context.show_world_cells)
   {
      render_world_cells(G_SCENE_INFO.camera);
   }

   if(Context.show_lightbulbs)
   {
      render_lightbulbs(G_SCENE_INFO.camera);
   }
   
   // render triaxis
   auto triaxis_view = glm::lookAt(vec3(0.0f), G_SCENE_INFO.camera->Front, -1.0f * G_SCENE_INFO.camera->Up);
   float displacement_x[3] = {0.3f, 0.0f, 0.0f};
   float displacement_y[3] = {0.0f, 0.3f, 0.0f};
   for(int i=0; i < 3; i++)
   {
      // ref. axis
	   auto axis = Context.tri_axis[i];
      axis->shader->use();
      axis->shader->setMatrix4("model", axis->matModel);
      axis->shader->setMatrix4("view", triaxis_view);
      axis->shader->setFloat2("screenPos", TRIAXIS_SCREENPOS_X, TRIAXIS_SCREENPOS_Y);
      render_entity(axis);
   }

   // --------------
   // render panels
   // --------------
   if(Context.world_panel.active)
   {
      render_world_panel(&Context.world_panel, world, player);
   }

   if(Context.entity_panel.active)
   {
      render_entity_panel(&Context.entity_panel);
      render_entity_control_arrows(&Context.entity_panel);
   }

   if(Context.palette_panel.active)
   {
      render_palette_panel(&Context.palette_panel);
   }

   if(Context.lights_panel.active)
   {
      render_lights_panel(&Context.lights_panel);
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

   if(Context.locate_coords_mode && Context.locate_coords_found_point)
   {
      G_IMMEDIATE_DRAW.add_point(Context.locate_coords_position, 2.0);
   }

   render_toolbar();

   render_text_overlay(player);

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void render_toolbar()
{
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 230, 180), ImGuiCond_Appearing);
   ImGui::Begin("Tools", &Context.toolbar_active, ImGuiWindowFlags_AlwaysAutoResize);

   string scene_name = "Scene name: " + G_SCENE_INFO.scene_name;
   ImGui::Text(scene_name.c_str());
   ImGui::NewLine();

   ImGui::InputFloat("##timestep", &G_FRAME_INFO.time_step, 0.5, 1.0, "Timestep = %.1f x");

   ImGui::NewLine();

   // GLOBAL CONFIGS
   {
      bool track = false;

      ImGui::Text("Cam speed");
      ImGui::DragFloat("##camspeed", &G_SCENE_INFO.camera->Acceleration, 0.5, 1, MAX_FLOAT);
      track = track || ImGui::IsItemDeactivatedAfterEdit();
      
      // Ambient light control
      ImGui::Text("Ambient light");
      auto ambient = G_SCENE_INFO.active_scene->ambient_light;
      float colors[3] = { ambient.x, ambient.y, ambient.z};
      if(ImGui::ColorEdit3("##ambient-color", colors))
      {
         G_SCENE_INFO.active_scene->ambient_light = vec3{colors[0], colors[1], colors[2]};
      }
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      ImGui::SliderFloat("##ambient-intensity", &G_SCENE_INFO.active_scene->ambient_intensity, 0, 1, "intensity = %.2f");
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      // save to file changes in config variables
      if(track)
      {
         G_CONFIG.camspeed = G_SCENE_INFO.camera->Acceleration;
         G_CONFIG.ambient_intensity = G_SCENE_INFO.active_scene->ambient_intensity;
         G_CONFIG.ambient_light = G_SCENE_INFO.active_scene->ambient_light;
         save_configs_to_file();
      }

      ImGui::NewLine();
   }

   // PANELS
   if(ImGui::Button("Entity Palette", ImVec2(150,18)))
   {
      Context.palette_panel.active = true;
   }

   if(ImGui::Button("World Panel", ImVec2(150,18)))
   {
      Context.world_panel.active = true;
      Context.show_world_cells = true;
   }

   if(ImGui::Button("Lights Panel", ImVec2(150,18)))
   {
      Context.lights_panel.active = true;
      Context.show_lightbulbs = true;
   }

   ImGui::NewLine();

   // TOOLS
   ImGui::Text("Measure");
   if(ImGui::Button("Y", ImVec2(40,18)))
   {
      activate_measure_mode();
   }

   ImGui::SameLine();
   if(ImGui::Button("X", ImVec2(40,18)))
   {
      
   }

   ImGui::SameLine();
   if(ImGui::Button("Z", ImVec2(40,18)))
   {

   }
   
   ImGui::NewLine();
   if(ImGui::Button("Locate Coordinates", ImVec2(150,18)))
   {
      activate_locate_coords_mode();
   }


   // SHOW STUFF
   ImGui::Checkbox("Show Event Triggers", &Context.show_event_triggers);
   ImGui::Checkbox("Show WorldStruct Cells", &Context.show_world_cells);
   ImGui::Checkbox("Show Point Lights", &Context.show_lightbulbs);
   ImGui::NewLine();

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

   auto shader = Shader_Catalogue.find("ortho_gui")->second;
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

   // palette panel
   initialize_palette(&Context.palette_panel);

   Context.last_frame_scene = G_SCENE_INFO.scene_name;
}


void render_text_overlay(Player* player)
{
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;
   float SCREEN_HEIGHT = G_DISPLAY_INFO.VIEWPORT_HEIGHT;

   string font = "consola14";
   string font_center = "swanseait38";
   string font_center_small = "swanseait20";
   vec3 tool_text_color_yellow = vec3(0.8, 0.8, 0.2);
   vec3 tool_text_color_green  = vec3(0.6, 1.0, 0.3);


   // CAMERA POSITION
   auto camera = G_SCENE_INFO.camera;

   string cam_p[3]{
      format_float_tostr(camera->Position.x, 2),
      format_float_tostr(camera->Position.y, 2), 
      format_float_tostr(camera->Position.z ,2), 
   };
   string camera_position  = "camera:   x: " + cam_p[0] + " y:" + cam_p[1] + " z:" + cam_p[2];
   render_text(font, 235, 45, camera_position);


   // PLAYER POSITION
   vec3 p_feet = player->feet();
   string player_p[3]{
      format_float_tostr(p_feet.x, 1),
      format_float_tostr(p_feet.y, 1),
      format_float_tostr(p_feet.z, 1),
   };
   string player_pos = "player:   x: " +  player_p[0] + " y: " +  player_p[1] + " z: " +  player_p[2];
   render_text(font, 235, 70, player_pos);


   // PLAYER LIVES
   string lives = to_string(player->lives);
   render_text(
      font,
      G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 
      90,
      player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1},
      lives
   );


   // PLAYER STATE
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

   string player_floor = "player floor: ";
   if(player->standing_entity_ptr != NULL)
      player_floor += player->standing_entity_ptr->name;
   render_text(G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 60, player_floor);
   render_text(G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 30, player_state_text_color, player_state_text);


   // FPS
   string fps = to_string(G_FRAME_INFO.current_fps);
   string fps_gui = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);
   render_text(font, G_DISPLAY_INFO.VIEWPORT_WIDTH - 120, 30, fps_gui);


   // EDITOR TOOLS INDICATORS

   // ----------
   // SNAP MODE
   // ----------
   float centered_text_height = SCREEN_HEIGHT - 120;
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

      // if position is changed and not commited, render text yellow
      vec3 snap_mode_subtext_color;
      if(Context.snap_reference == nullptr)
         snap_mode_subtext_color = tool_text_color_yellow;
      else
      {
         auto state = Context.undo_stack.check();  
         if(state.entity != nullptr && state.position != Context.entity_panel.entity->position)
            snap_mode_subtext_color = tool_text_color_yellow;
         else
            snap_mode_subtext_color = tool_text_color_green;
      }

      // selects text based on situation of snap tool
      string sub_text;
      if(Context.snap_reference == nullptr)
         sub_text = "select another entity to snap to.";
      else
         sub_text = "press Enter to commit position. x/y/z to change axis.";
      
      render_text(
         font_center, 
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
         centered_text_height, 
         tool_text_color_yellow, 
         true,
         "SNAP MODE (" + snap_axis + "-" + snap_cycle + ")"
      );

       render_text(
         font_center_small, 
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
         centered_text_height - 40, 
         snap_mode_subtext_color, 
         true,
         sub_text
      );
   }

   // -------------
   // MEASURE MODE
   // -------------
   if(Context.measure_mode)
   {
      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "MEASURE MODE (Y)"
      );

      if(Context.second_point_found)
      {
         render_text(
            font_center,
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
            centered_text_height - 40,
            vec3(0.8, 0.8, 0.2),
            true,
            "(" + format_float_tostr(abs(Context.measure_to - Context.measure_from.y), 2) + " m)"
         ); 
      }
   }

   // ----------
   // MOVE MODE
   // ----------
   if(Context.move_mode)
   {
      string move_axis;
      switch(Context.move_axis)
      {
         case 0:
            move_axis = "XZ";
            break;
         case 1:
            move_axis = "X";
            break;
         case 2:
            move_axis = "Y";
            break;
         case 3:
            move_axis = "Z";
            break;
      }

      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "MOVE MODE (" + move_axis + ")"
      );
   }

   // -------------------
   // LOCATE COORDS MODE
   // -------------------
   if(Context.locate_coords_mode)
   {
      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "LOCATE COORDS MODE"
      );

      string locate_coords_subtext;
      if(!Context.locate_coords_found_point)
      {
         locate_coords_subtext = "Please select a world position to get coordinates.";
      }
      else
      {
         locate_coords_subtext = 
            "(x: "  + format_float_tostr(Context.locate_coords_position[0], 2) +
            ", y: " + format_float_tostr(Context.locate_coords_position[1], 2) +
            ", z: " + format_float_tostr(Context.locate_coords_position[2], 2) + ")";
      }

      render_text(
         font_center_small, 
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
         centered_text_height - 40, 
         tool_text_color_green, 
         true,
         locate_coords_subtext
      );
   }


}

void render_event_triggers(Camera* camera)
{
   auto checkpoints = G_SCENE_INFO.active_scene->checkpoints;
   if(checkpoints.size() == 0)
      return;
      
   auto find = Shader_Catalogue.find("color");
   auto shader = find->second;

   for(int i = 0; i < checkpoints.size(); i++)
   {
      auto checkpoint = checkpoints[i];
      shader->use();
      shader->setFloat3("color", 0.5, 0.5, 0.3);
      shader->setFloat("opacity", 0.6);
      shader->setMatrix4("model", checkpoint->trigger_model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(checkpoint->trigger, RenderOptions{});
   }
}

void render_world_cells(Camera* camera)
{
   auto& scene = G_SCENE_INFO.active_scene;

   auto shader = Shader_Catalogue.find("color")->second;
   auto cell_mesh = Geometry_Catalogue.find("world cell")->second;
   
   for(int i = 0; i < World.cells_in_use_count; i++)
   {
      RenderOptions opts;
      opts.wireframe = true;

      auto cell = World.cells_in_use[i];

      vec3 color;
      if(Context.world_panel.cell_coords.x == cell->i &&
         Context.world_panel.cell_coords.y == cell->j &&
         Context.world_panel.cell_coords.z == cell->k)
      {
         opts.line_width = 1.5;
         color = vec3(0.8, 0.4, 0.2);
      }
      else if((cell->i == W_CELLS_NUM_X || cell->i == 0) ||
              (cell->j == W_CELLS_NUM_Y || cell->j == 0) ||
              (cell->k == W_CELLS_NUM_Z || cell->k == 0))
      {
         color = vec3(0.0, 0.0, 0.0);
      }
      else color = vec3(0.27, 0.55, 0.65);

      // creates model matrix
      vec3 position = get_world_coordinates_from_world_cell_coordinates(
         cell->i, cell->j, cell->k
      );
      glm::mat4 model = translate(mat4identity, position);
		model = glm::scale(model, vec3{W_CELL_LEN_METERS, W_CELL_LEN_METERS, W_CELL_LEN_METERS});

      //render
      shader->use();
      shader->setFloat3("color", color);
      shader->setFloat("opacity", 0.85);
      shader->setMatrix4("model", model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(cell_mesh, opts);
   }
}

void render_lightbulbs(Camera* camera)
{
   auto& scene = G_SCENE_INFO.active_scene;
   auto mesh = Geometry_Catalogue.find("lightbulb")->second;
   auto shader = Shader_Catalogue.find("color")->second;

   auto selected_light = Context.lights_panel.selected_light;
   auto selected_light_type = Context.lights_panel.selected_light_type;

   // point lights
   int point_c = 0;
   for(auto const& light: scene->pointLights)
   {
      auto model = translate(mat4identity, light.position + vec3{0, 0.5, 0});
      model = glm::scale(model, vec3{0.1f});
      RenderOptions opts;
      //opts.wireframe = true;
      //render
      shader->use();
      shader->setFloat3("color", light.diffuse);
      shader->setFloat("opacity", 1.0);
      shader->setMatrix4("model", model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(mesh, opts);

      point_c++;
   }

   // spot lights
   int spot_c = 0;
   for(auto const& light: scene->spotLights)
   {
      auto model = translate(mat4identity, light.position + vec3{0, 0.5, 0});
      model = glm::scale(model, vec3{0.1f});
      RenderOptions opts;
      //opts.wireframe = true;
      //render
      shader->use();
      shader->setFloat3("color", light.diffuse);
      shader->setFloat("opacity", 1.0);
      shader->setMatrix4("model", model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(mesh, opts);

      spot_c++;
   }

   // directional lights
   int directional_c = 0;
   for(auto const& light: scene->directionalLights)
   {
      auto model = translate(mat4identity, light.position + vec3{0, 0.5, 0});
      model = glm::scale(model, vec3{0.1f});
      RenderOptions opts;
      //opts.wireframe = true;
      //render
      shader->use();
      shader->setFloat3("color", light.diffuse);
      shader->setFloat("opacity", 1.0);
      shader->setMatrix4("model", model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(mesh, opts);

      directional_c++;
   }

   // render selection box and dir arrow for selected lightbulb
   if(selected_light >= 0)
   {
      vec3 light_position;
      vec3 light_direction;
      if(selected_light_type == "point")
      {
         assert(selected_light < point_c);
         auto light = scene->pointLights[selected_light];
         light_position = light.position;
      }
      else if(selected_light_type == "spot")
      {
         assert(selected_light < spot_c);
         auto light = scene->spotLights[selected_light];
         light_position = light.position;
         light_direction = light.direction;
      }
      else if(selected_light_type == "directional")
      {
         assert(selected_light < directional_c);
         auto light = scene->directionalLights[selected_light];
         light_position = light.position;
         light_direction = light.direction;
      }

      // selection box
      auto aabb_mesh = Geometry_Catalogue.find("aabb")->second;

      auto aabb_model = translate(mat4identity, light_position - vec3{0.1575, 0, 0.1575});
      aabb_model = glm::scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});
      RenderOptions opts;
      opts.wireframe = true;

      shader->use();
      shader->setFloat3("color", vec3{0.9, 0.7, 0.9});
      shader->setFloat("opacity", 1.0);
      shader->setMatrix4("model", aabb_model);
      shader->setMatrix4("view", camera->View4x4);
      shader->setMatrix4("projection", camera->Projection4x4);
      render_mesh(aabb_mesh, opts);

      // direction arrow
      if (selected_light_type == "spot" || selected_light_type == "directional")
      {
         float pitch, yaw;
         compute_angles_from_direction(pitch, yaw, light_direction);
         vec3 arrow_direction = compute_direction_from_angles(pitch, yaw);

         vec3 arrow_origin = light_position - vec3{0.0, 0.56, 0.0};
         vec3 arrow_end = arrow_origin + arrow_direction * 1.5f;
         vec3 points[2]{ arrow_origin, arrow_end };
         G_IMMEDIATE_DRAW.add_line(points, 1.5);
      }

      // @todo: epic fail below (trying to rotate an arrow mesh according to a dir vector)
      // auto arrow_mesh = Geometry_Catalogue.find("axis")->second;
      // vec3 front = arrow_origin + light.direction;
      // vec3 up = glm::cross(arrow_origin, );

      // @todo: this is a workaround, since we are not using quaternions yet, we must
      //       be careful with 0/180 degree angles between up and direction vectors
      //       using glm::lookAt()
      // @todo: Actually now we are using immediate draw and lines.

      // //mat4 arrow_model = translate(mat4identity, arrow_origin);
      // mat4 arrow_model = 
      //    glm::translate(mat4identity, arrow_origin) *
      //    glm::rotate(mat4identity, glm::radians(90.0f), vec3(1, 0, 0)) *
      //    glm::lookAt(vec3{0.0}, arrow_direction, vec3{0,1,0})
      // ;
      // //arrow_model = glm::scale(arrow_model, vec3{0.2f, 0.3f, 0.2f});

      //render arrow
      // shader->use();
      // shader->setFloat3("color", vec3{0.9, 0.7, 0.9});
      // shader->setFloat("opacity", 1.0);
      // shader->setMatrix4("model", arrow_model);
      // shader->setMatrix4("view", camera->View4x4);
      // shader->setMatrix4("projection", camera->Projection4x4);
      // render_mesh(arrow_mesh, RenderOptions{});
   }
}

void check_selection_to_open_panel()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   auto test_light = test_ray_against_lights(pickray);
   if(test.hit && (!test_light.hit || test_light.distance > test.distance))
      open_entity_panel(test.entity);
   else if(test_light.hit)
      open_lights_panel(test_light.obj_hit_type, test_light.obj_hit_index, true);
}

void check_selection_to_move_entity()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray, true);
   auto test_light = test_ray_against_lights(pickray);
   if(test.hit && (!test_light.hit || test_light.distance > test.distance))
      activate_move_mode(test.entity);
   else if(test_light.hit)
      activate_move_light_mode(test_light.obj_hit_type, test_light.obj_hit_index);
}

}
