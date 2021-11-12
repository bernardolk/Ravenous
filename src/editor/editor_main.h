#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <editor/editor_colors.h>

namespace Editor
{

const static string EDITOR_ASSETS = PROJECT_PATH + "/assets/editor/";

const static float TRIAXIS_SCREENPOS_X = -1.80;
const static float TRIAXIS_SCREENPOS_Y = -1.80;

#include <editor/editor_undo.h>
#include <editor/editor_panel_contexts.h>

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
   PlayerPanelContext player_panel;
   WorldPanelContext world_panel;
   PalettePanelContext palette_panel;
   LightsPanelContext lights_panel;
   CollisionLogPanelContext collision_log_panel;
   InputRecorderPanelContext input_recorder_panel;

   // toolbar
   bool toolbar_active = true;

   // general mode controls
   bool mouse_click = false;
   Entity* selected_entity = nullptr;

   // move mode
   bool move_mode = false;
   bool scale_on_drop = false;
   u8 move_axis = 0;

   // place mode
   bool place_mode = false;

   // move light @todo: will disappear!
   string selected_light_type = "";
   int selected_light = -1;

   // scale mode
   bool scale_entity_with_mouse = false;
   
   // measure mode
   bool measure_mode = false;
   u8 measure_axis = 0;          // x,y,z == 0,1,2
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
   bool snap_inside = false;
   Entity* snap_reference = nullptr;
   EntityState snap_tracked_state;

   // stretch mode
   bool stretch_mode = false;

   // show things 
   bool show_event_triggers = false;
   bool show_world_cells = false;
   bool show_lightbulbs = true;

   // gizmos
   Entity* tri_axis[3];
   Entity* tri_axis_letters[3];
} EdContext;


void initialize();
void update();
void render(Player* player, WorldStruct* world);
void terminate();

void update_editor_entities();
void check_selection_to_open_panel(Player* player);
void check_selection_to_move_entity();

void render_text_overlay(Player* player);
void render_event_triggers(Camera* camera);
void render_world_cells(Camera* camera);
void render_lightbulbs(Camera* camera);
void start_dear_imgui_frame();
void end_dear_imgui_frame();

#include <editor/editor_tools.h>
#include <editor/editor_toolbar.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_player_panel.h>
#include <editor/editor_world_panel.h>
#include <editor/editor_input.h>
#include <editor/editor_palette_panel.h>
#include <editor/editor_lights_panel.h>
#include <editor/editor_collision_log_panel.h>
#include <editor/editor_input_recorder_panel.h>

//------------------
// > UPDATE EDITOR
//------------------

void update()
{
   if(EdContext.last_frame_scene != G_SCENE_INFO.scene_name)
   {
      EdContext.entity_panel.active = false;
      EdContext.world_panel.active = false;
   }

   EdContext.last_frame_scene = G_SCENE_INFO.scene_name;

   // check for asset changes
   check_for_asset_changes();
   update_editor_entities();

   // ENTITY PANEL
   if(EdContext.entity_panel.active)
   {
      update_entity_control_arrows(&EdContext.entity_panel);
   }
   else
   {
      EdContext.entity_panel.rename_buffer[0] = 0;
      EdContext.snap_mode = false;
      EdContext.stretch_mode = false;
      EdContext.snap_reference = nullptr;
   }

   // unselect lights when not panel is not active
   if(!EdContext.lights_panel.active)
   {
      EdContext.lights_panel.selected_light = -1;
      EdContext.lights_panel.selected_light_type = "";
   }
   else if(
      EdContext.lights_panel.selected_light != -1 &&
      EdContext.lights_panel.selected_light_type != ""
   )
   {
      EdContext.show_lightbulbs = true;
   }


   // set editor mode values to initial if not active
   if(!EdContext.measure_mode)
   {
      EdContext.first_point_found  = false;
      EdContext.second_point_found = false;
   }
   if(!EdContext.snap_mode)
   {
      EdContext.snap_cycle = 0;
      EdContext.snap_axis = 1;
      EdContext.snap_reference = nullptr;
   }

   // respond to mouse if necessary
   if(EdContext.move_mode)
   {
      if(EdContext.mouse_click)
      {
         if(EdContext.selected_light > -1)
            place_light();
         else
            place_entity();
      }
      else
      {
         if(EdContext.selected_light > -1)
            move_light_with_mouse(EdContext.selected_light_type, EdContext.selected_light);
         else
            move_entity_with_mouse(EdContext.selected_entity);
      }
   }

   if(EdContext.place_mode)
   {
      if(EdContext.mouse_click)
      {
         place_entity();
      }
      else
         place_entity_with_mouse(EdContext.selected_entity);
   }

   if(EdContext.scale_entity_with_mouse)
   {
      scale_entity_with_mouse(EdContext.selected_entity);
   }

   // resets mouse click event
   EdContext.mouse_click = false;
}

void update_editor_entities()
{
   for(int i=0; i < 3; i++)
   {
	   auto entity = EdContext.tri_axis[i];
      glm::mat4 model = mat4identity;
		model = glm::rotate(model, glm::radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, entity->scale);
		entity->matModel = model;
   }
}


//---------------------
// > RENDER EDITOR UI
//---------------------

void render(Player* player, WorldStruct* world)
{
   // render world objs if toggled
   if(EdContext.show_event_triggers)
   {
      render_event_triggers(G_SCENE_INFO.camera);
   }

   if(EdContext.show_world_cells)
   {
      render_world_cells(G_SCENE_INFO.camera);
   }

   if(EdContext.show_lightbulbs)
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
	   auto axis = EdContext.tri_axis[i];
      axis->shader->use();
      axis->shader->setMatrix4("model", axis->matModel);
      axis->shader->setMatrix4("view", triaxis_view);
      axis->shader->setFloat2("screenPos", TRIAXIS_SCREENPOS_X, TRIAXIS_SCREENPOS_Y);
      render_entity(axis);
   }

   // render glowing wireframe on top of selected entity
   if(EdContext.entity_panel.active)
   {
      // update
      auto state = get_entity_state(EdContext.selected_entity);
      auto model = mat_model_from_entity_state(state);

      // compute color intensity based on time
      float time_value = glfwGetTime();
      float intensity = sin(time_value) * 2;
      if(intensity < 0) intensity *= -1.0;
      intensity += 1.0;

      // render
      auto glowing_line = Shader_Catalogue.find("color")->second;
      glowing_line->use();
      glowing_line->setMatrix4("model", model);
      glowing_line->setFloat3("color", intensity * 0.890, intensity * 0.168, intensity * 0.6);
      glowing_line->setFloat("opacity", 1);
      render_mesh(EdContext.selected_entity->mesh, RenderOptions{true, false, 3});
   }

   // render glowing wireframe on top of snap reference entity
   if(EdContext.snap_mode && EdContext.snap_reference != nullptr)
   {
      // update
      auto state = get_entity_state(EdContext.snap_reference);
      auto model = mat_model_from_entity_state(state);

      // compute color intensity based on time
      float time_value = glfwGetTime();
      float intensity = sin(time_value) * 2;
      if(intensity < 0) intensity *= -1.0;
      intensity += 1.0;

      // render
      auto glowing_line = Shader_Catalogue.find("color")->second;
      glowing_line->use();
      glowing_line->setMatrix4("model", model);
      glowing_line->setFloat3("color", intensity * 0.952, intensity * 0.843, intensity * 0.105);
      glowing_line->setFloat("opacity", 1);
      render_mesh(EdContext.snap_reference->mesh, RenderOptions{true, false, 3});
   }

   // --------------
   // render panels
   // --------------
   if(EdContext.world_panel.active)
      render_world_panel(&EdContext.world_panel, world, player);

   if(EdContext.entity_panel.active)
   {
      auto& panel = EdContext.entity_panel;

      render_entity_panel(&panel);
      render_entity_control_arrows(&panel);

      if(panel.show_normals)
         render_entity_mesh_normals(&panel);
      if(panel.show_collider)
         IM_RENDER.add_mesh(IMHASH, &panel.entity->collider, COLOR_PURPLE_1, 0);
      if(panel.show_bounding_box)
      {
         auto aabb = Geometry_Catalogue.find("aabb")->second;
         auto [pos, scale] = panel.entity->bounding_box.get_pos_and_scale();
         IM_RENDER.add_mesh(IMHASH, aabb, pos, vec3(0), scale, COLOR_PINK_1, 0);
      }
   }

   if(EdContext.player_panel.active)
   {
      render_player_panel(&EdContext.player_panel);
   }

   if(EdContext.palette_panel.active)
      render_palette_panel(&EdContext.palette_panel);

   if(EdContext.lights_panel.active)
      render_lights_panel(&EdContext.lights_panel);

   if(EdContext.input_recorder_panel.active)
      render_input_recorder_panel(&EdContext.input_recorder_panel);

   if(EdContext.collision_log_panel.active)
      render_collision_log_panel(&EdContext.collision_log_panel);

   // -----------------------
   // render gizmos inscreen
   // -----------------------
   if(EdContext.measure_mode && EdContext.first_point_found && EdContext.second_point_found)
   {
      auto render_opts = RenderOptions();
      render_opts.always_on_top = true;
      render_opts.line_width = 2.0;
      render_opts.color = ED_RED;

      vec3 second_point;
      if(EdContext.measure_axis == 0)
         second_point = vec3(EdContext.measure_to, EdContext.measure_from.y, EdContext.measure_from.z);
      if(EdContext.measure_axis == 1)
         second_point = vec3(EdContext.measure_from.x, EdContext.measure_to, EdContext.measure_from.z);
      if(EdContext.measure_axis == 2)
         second_point = vec3(EdContext.measure_from.x, EdContext.measure_from.y, EdContext.measure_to);   

      IM_RENDER.add(
         IMHASH,
         vector<Vertex>{
            Vertex{EdContext.measure_from},
            Vertex{second_point}
         },
         GL_LINE_LOOP,
         render_opts
      );
   }

   if(EdContext.locate_coords_mode && EdContext.locate_coords_found_point)
   {
      IM_RENDER.add_point(IMHASH, EdContext.locate_coords_position, 2.0);
   }

   render_toolbar();

   render_text_overlay(player);

   ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
	EdContext.imStyle = &ImGui::GetStyle();
	EdContext.imStyle->WindowRounding = 1.0f;

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

   EdContext.tri_axis[0] = x_axis;
   EdContext.tri_axis[1] = y_axis;
   EdContext.tri_axis[2] = z_axis;

   auto letter_x_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_x");
   auto x_axis_letter = new Entity();
   x_axis_letter->mesh = letter_x_mesh;
   x_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "blue.jpg",  "blue axis"});
   x_axis_letter->shader = shader;
   x_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   EdContext.tri_axis_letters[0] = x_axis_letter;

   auto letter_y_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_y");
   auto y_axis_letter = new Entity();
   y_axis_letter->mesh = letter_y_mesh;
   y_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "green.jpg",  "green axis"});
   y_axis_letter->shader = shader;
   y_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   EdContext.tri_axis_letters[1] = y_axis_letter;

   auto letter_z_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "letter_z");
   auto z_axis_letter = new Entity();
   z_axis_letter->mesh = letter_z_mesh;
   z_axis_letter->textures.push_back(Texture{blue_tex,  "texture_diffuse", "pink.jpg",  "pink axis"});
   z_axis_letter->shader = shader;
   z_axis_letter->scale = vec3{0.1, 0.1, 0.1};
   EdContext.tri_axis_letters[2] = z_axis_letter;

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

   EdContext.entity_panel.x_arrow = x_arrow;
   EdContext.entity_panel.y_arrow = y_arrow;
   EdContext.entity_panel.z_arrow = z_arrow;

   // palette panel
   initialize_palette(&EdContext.palette_panel);

   EdContext.last_frame_scene = G_SCENE_INFO.scene_name;
}


void render_text_overlay(Player* player)
{
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;
   float SCREEN_HEIGHT = G_DISPLAY_INFO.VIEWPORT_HEIGHT;

   string font = "consola14";
   string font_center = "swanseait38";
   string font_center_small = "swanseait20";
   float centered_text_height = SCREEN_HEIGHT - 120;
   float centered_text_height_small = centered_text_height - 40;
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
   vec3 player_state_text_color = vec3(0, 0, 0);
   std::string player_state_text;
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
         player_state_text = "PLAYER STANDING";
         break;
      case PLAYER_STATE_FALLING:
         player_state_text = "PLAYER FALLING";
         break;
      case PLAYER_STATE_FALLING_FROM_EDGE:
         player_state_text = "PLAYER FALLING FROM EDGE";
         break;
      case PLAYER_STATE_JUMPING:
         player_state_text = "PLAYER JUMPING";
         break;
      case PLAYER_STATE_SLIDING:
         player_state_text = "PLAYER SLIDING";
         break;
      case PLAYER_STATE_SLIDE_FALLING:
         player_state_text = "PLAYER SLIDE FALLING";
         break;
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
         player_state_text = "PLAYER EVICTED FROM SLOPE";
         break;
   }
   render_text("consola18", G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 30, player_state_text_color, player_state_text);


   string player_floor = "player floor: ";
   if(player->standing_entity_ptr != NULL)
      player_floor += player->standing_entity_ptr->name;
   render_text(G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 60, player_floor);

   string p_grab = "grabbing: ";
   if(player->grabbing_entity != NULL)
      p_grab += player->grabbing_entity->name;
   render_text(G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 45, p_grab);


   // FPS
   string fps = to_string(G_FRAME_INFO.current_fps);
   string fps_gui = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);
   render_text(font, G_DISPLAY_INFO.VIEWPORT_WIDTH - 120, 30, fps_gui);


   // EDITOR TOOLS INDICATORS

   // ----------
   // SNAP MODE
   // ----------
   if(EdContext.snap_mode)
   {
      string snap_cycle;
      switch(EdContext.snap_cycle)
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
      switch(EdContext.snap_axis)
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
      if(EdContext.snap_reference == nullptr)
         snap_mode_subtext_color = tool_text_color_yellow;
      else
      {
         auto state = EdContext.undo_stack.check();
         if(state.entity != nullptr && state.position != EdContext.entity_panel.entity->position)
            snap_mode_subtext_color = tool_text_color_yellow;
         else
            snap_mode_subtext_color = tool_text_color_green;
      }

      // selects text based on situation of snap tool
      string sub_text;
      if(EdContext.snap_reference == nullptr)
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
         centered_text_height_small, 
         snap_mode_subtext_color, 
         true,
         sub_text
      );
   }

   // -------------
   // MEASURE MODE
   // -------------
   if(EdContext.measure_mode)
   {
      string axis = 
         EdContext.measure_axis == 0 ? "x" :
         EdContext.measure_axis == 1 ? "y" :
         "z";

      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "MEASURE MODE (" + axis + ")"
      );

      if(EdContext.second_point_found)
      {
         float dist_ref = 
            EdContext.measure_axis == 0 ? EdContext.measure_from.x :
            EdContext.measure_axis == 1 ? EdContext.measure_from.y :
                                        EdContext.measure_from.z ;

         render_text(
            font_center,
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
            centered_text_height_small,
            vec3(0.8, 0.8, 0.2),
            true,
            "(" + format_float_tostr(abs(EdContext.measure_to - dist_ref), 2) + " m)"
         ); 
      }
   }

   // ----------
   // MOVE MODE
   // ----------
   if(EdContext.move_mode)
   {
      string move_axis;
      switch(EdContext.move_axis)
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

      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height_small,
         vec3(0.8, 0.8, 0.2),
         true,
         "press M to alternate between move and place modes"
      );
   }

   // ----------
   // PLACE MODE
   // ----------
   if(EdContext.place_mode)
   {
      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "PLACE MODE"
      );

      render_text(
         font_center_small,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height_small,
         vec3(0.8, 0.8, 0.2),
         true,
         "press M to alternate between move and place modes"
      );
   }

   // -------------------
   // LOCATE COORDS MODE
   // -------------------
   if(EdContext.locate_coords_mode)
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
      if(!EdContext.locate_coords_found_point)
      {
         locate_coords_subtext = "Please select a world position to get coordinates.";
      }
      else
      {
         locate_coords_subtext = 
            "(x: "  + format_float_tostr(EdContext.locate_coords_position[0], 2) +
            ", y: " + format_float_tostr(EdContext.locate_coords_position[1], 2) +
            ", z: " + format_float_tostr(EdContext.locate_coords_position[2], 2) + ")";
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

   // -------------
   // STRETCH MODE
   // -------------
   if(EdContext.stretch_mode)
   {
      render_text(
         font_center,
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2,
         centered_text_height,
         vec3(0.8, 0.8, 0.2),
         true,
         "STRETCH MODE"
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
      if(EdContext.world_panel.cell_coords.x == cell->i &&
         EdContext.world_panel.cell_coords.y == cell->j &&
         EdContext.world_panel.cell_coords.z == cell->k)
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
      glDisable(GL_CULL_FACE);
      render_mesh(cell_mesh, opts);
      glEnable(GL_CULL_FACE);
   }
}


void render_lightbulbs(Camera* camera)
{
   auto& scene = G_SCENE_INFO.active_scene;
   auto mesh = Geometry_Catalogue.find("lightbulb")->second;
   auto shader = Shader_Catalogue.find("color")->second;

   auto selected_light = EdContext.lights_panel.selected_light;
   auto selected_light_type = EdContext.lights_panel.selected_light_type;

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

   // render selection box and dir arrow for selected lightbulb
   if(selected_light >= 0)
   {
      vec3 light_position;
      vec3 light_direction;
      if(selected_light_type == "point")
      {
         assert(selected_light <= point_c);
         auto light = scene->pointLights[selected_light];
         light_position = light.position;
      }
      else if(selected_light_type == "spot")
      {
         assert(selected_light <= spot_c);
         auto light = scene->spotLights[selected_light];
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
      if (selected_light_type == "spot")
      {
         float pitch, yaw;
         compute_angles_from_direction(pitch, yaw, light_direction);
         vec3 arrow_direction = compute_direction_from_angles(pitch, yaw);

         vec3 arrow_origin = light_position - vec3{0.0, 0.56, 0.0};
         vec3 arrow_end = arrow_origin + arrow_direction * 1.5f;
         IM_RENDER.add_line(IMHASH, arrow_origin, arrow_end, 1.5);
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

void check_selection_to_open_panel(Player* player)
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray, true);
   auto test_light = test_ray_against_lights(pickray);
   if(test.hit && (!test_light.hit || test_light.distance > test.distance))
   {
      if(test.entity->name == PLAYER_NAME)
         open_player_panel(player);
      else
         open_entity_panel(test.entity);

   }
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

void start_dear_imgui_frame()
{
   ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void end_dear_imgui_frame()
{
	ImGui::EndFrame();
}

}
