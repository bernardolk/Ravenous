
struct InputRecorderPanelContext {
   bool active = false;
   int selected_recording = -1;
};

struct PalettePanelContext {
   bool active = true;
   unsigned int textures[15];
   EntityAttributes entity_palette[15];
   unsigned int count = 0;
};

struct EntityPanelContext {
   bool active = false;
   bool focused = false;
   Entity* entity = nullptr;
   vec3 original_position = vec3(0);
   vec3 original_scale = vec3(0);
   float original_rotation = 0;
   
   //rename buffer
   bool rename_option_active = false;
   const static size_t _rename_buff_size = 100;
   char rename_buffer[_rename_buff_size];

   bool reverse_scale   = false;
   bool reverse_scale_x = false;
   bool reverse_scale_y = false;
   bool reverse_scale_z = false;
   Entity* x_arrow;
   Entity* y_arrow;
   Entity* z_arrow;
   EntityState entity_tracked_state;
   bool show_normals       = false;
   bool show_collider      = false;
   bool show_bounding_box  = false;

   void empty_rename_buffer()
   {
      for(int i = 0; i < _rename_buff_size; i++)
         rename_buffer[i] = 0;
   }

   bool validate_rename_buffer_contents()
   {
      for(int i = 0; i < _rename_buff_size; i++)
      {
         auto cursor = rename_buffer[i];
         if(cursor == '\0')
            return true;
         if(cursor == ' ' || cursor == 0)
            return false;
      }

      cout << "Invalid c-string in rename buffer.\n";
      assert(false);
      return false;
   }
};

struct PlayerPanelContext {
   bool active = false;
   bool focused = false;
   Player* player;
};

struct WorldPanelContext {
   bool active = false;
   vec3 cell_coords = vec3{-1.0f};
};

struct LightsPanelContext {
   bool active = false;
   bool focused = false;
   bool focus_tab = false;

   // selected light
   int selected_light = -1;
   float selected_light_yaw;
   float selected_light_pitch;
   string selected_light_type;
};

struct CollisionLogPanelContext {
   bool active = false;
   bool focused = false;
};