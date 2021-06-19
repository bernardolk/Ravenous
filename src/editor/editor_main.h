#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Editor
{
const static string EDITOR_ASSETS = PROJECT_PATH + "/assets/editor/";

const static float TRIAXIS_SCREENPOS_X = -1.80;
const static float TRIAXIS_SCREENPOS_Y = -1.80;

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
};

struct WorldPanelContext {
   bool active = false;
   vec3 cell_coords = vec3{-1.0f};
};

struct EntityState {
   vec3 position;
   vec3 scale;
   vec3 rotation;
};

struct EditorContext {
   ImGuiStyle* imStyle;
   EntityPanelContext entity_panel;
   WorldPanelContext world_panel;
   PalettePanelContext palette_panel;

   // toolbar
   bool toolbar_active = true;

   // general mode controls
   bool mouse_click = false;
   Entity* selected_entity = nullptr;
   EntityState original_entity_state;

   // move mode
   bool move_mode = false;
   bool scale_on_drop = false;
   u8 move_axis = 0;

   // scale mode
   bool scale_entity_with_mouse = false;
   
   // measure mode
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
void render(Player* player, WorldStruct* world);
void render_text_overlay(Player* player);
void render_toolbar();
void render_event_triggers(Camera* camera);
void render_world_cells(Camera* camera);
void end_frame();
void terminate();

#include <editor/editor_tools.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_world_panel.h>
#include <editor/editor_input.h>
#include <editor/editor_palette_panel.h>


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
         // if entity is being moved from palette, on drop we get to scale it
         // if moving using the move shortcut, we don't
         if(Context.scale_on_drop)
            Context.scale_entity_with_mouse = true;
         else
            deselect_entity();
         
         auto update_cells = World.update_entity_world_cells(Context.selected_entity);
         if(update_cells.status != OK)
         {
            G_BUFFERS.rm_buffer->add(update_cells.message, 3500);
         }
         World.update_cells_in_use_list();
      }
      else move_entity_with_mouse(Context.selected_entity);
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

   // render panels if active
   if(Context.world_panel.active)
   {
      render_world_panel(&Context.world_panel, world);
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
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 230, 180), ImGuiCond_Appearing);
   ImGui::Begin("Tools", &Context.toolbar_active, ImGuiWindowFlags_AlwaysAutoResize);

   string scene_name = "Scene -> [" + G_SCENE_INFO.scene_name + "]";
   ImGui::Text(scene_name.c_str());

   ImGui::Text("Cam speed");
   if(ImGui::SliderFloat("", &G_SCENE_INFO.camera->Acceleration, 1, 16))
   {
      //@Todo: saving this all the time is not cool.
      //       could add an event to a datastructure with a timing info and
      //       an event identifier, then every time i insert an event there
      //       with the same id it will keep it alive longer, when it expires
      //       we execute it (save configs in this case).
      G_CONFIG.camspeed = G_SCENE_INFO.camera->Acceleration;
      save_configs_to_file();
   }

   if(ImGui::Button("Entity Palette", ImVec2(150,18)))
   {
      Context.palette_panel.active = true;
   }

   if(ImGui::Button("World Panel", ImVec2(150,18)))
   {
      Context.world_panel.active = true;
   }

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
   ImGui::Checkbox("Show Event Triggers", &Context.show_event_triggers);
   ImGui::Checkbox("Show WorldStruct Cells", &Context.show_world_cells);

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
}


void render_text_overlay(Player* player)
{

   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;
   float SCREEN_HEIGHT = G_DISPLAY_INFO.VIEWPORT_HEIGHT;

   string font = "consola14";
   string font_center = "swanseait38";


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
   string player_p[3]{
      format_float_tostr(player->entity_ptr->position.x, 1),
      format_float_tostr(player->entity_ptr->position.y, 1),
      format_float_tostr(player->entity_ptr->position.z, 1),
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

      vec3 snap_mode_color;
      if(Context.entity_state_before_snap.position != Context.entity_panel.entity->position)
         snap_mode_color = vec3(0.8, 0.8, 0.2);
      else
         snap_mode_color = vec3(0.6, 1.0, 0.3);

      render_text(
         font_center, 
         G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
         centered_text_height, 
         snap_mode_color, 
         true,
         "SNAP MODE (" + snap_axis + "-" + snap_cycle + ")"
      );
   }

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
            centered_text_height - 20,
            vec3(0.8, 0.8, 0.2),
            true,
            "(" + format_float_tostr(abs(Context.measure_to - Context.measure_from.y), 2) + " m)"
         ); 
      }
   }

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

}
