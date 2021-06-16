// catalogues 
std::map<string, Mesh*> Geometry_Catalogue;
std::map<string, Shader*> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;

typedef std::map<GLchar, Character> gl_charmap;
std::map<string, gl_charmap> Font_Catalogue;

// World
WorldStruct World;

struct GlobalSceneInfo {
   Scene* active_scene = NULL;
   Camera* camera;
   Camera* views[2];
   Player* player;
   bool input_mode = false;
   string scene_name;
} G_SCENE_INFO;

struct EntityBufferElement {
   Entity* entity;
   bool  collision_check = false;
};

struct EntityBuffer {
   EntityBufferElement* buffer;
   int size;
};

struct RenderMessageBufferElement {
   string message = "";
   float elapsed = 0;
   float duration = 0;
};

struct RenderMessageBuffer {
   RenderMessageBufferElement* buffer;
   size_t size;
   u16 count = 0;

   bool add(string msg, float duration)
   {
      if(count < size)
      {
         auto item = buffer;
         for(int i = 0; i < size; i++)
         {
            if(item->message == "")
            {
               item->message = msg;
               item->elapsed = 0;
               item->duration = duration;
               count++;
               break;
            }
            item++;
         }
         return true;
      }
      else
      {
         cout << "WARNING: message has not been addded to message buffer"
         << "because it is FULL. Message was: " << msg << "\n";
         return false;
      }
   }
};

struct GlobalBuffers {
   EntityBuffer* entity_buffer;
   RenderMessageBuffer* rm_buffer;
} G_BUFFERS;