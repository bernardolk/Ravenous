// catalogues 
std::map<string, Mesh*> Geometry_Catalogue;
std::map<string, Shader*> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;

typedef std::map<GLchar, Character> gl_charmap;
std::map<string, gl_charmap> Font_Catalogue;


size_t COLLISION_LOG_CAPACITY = 20;
size_t COLLISION_BUFFER_CAPACITY = WORLD_CELL_CAPACITY * 8;
size_t MESSAGE_BUFFER_CAPACITY = 10;

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

// collision log
struct CollisionLogEntry {
   Entity* entity;
   CollisionOutcomeEnum outcome;
   int iteration;
};

struct CollisionLog {
   CollisionLogEntry* entries;
   int size;
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
   CollisionLog* collision_log;
} G_BUFFERS;


void log_collision(CollisionData data, int iteration)
{
   int& size = G_BUFFERS.collision_log->size;
   if(size == COLLISION_LOG_CAPACITY)
   {
      G_BUFFERS.rm_buffer->add("Warning: Collision Log is full.", 3000);
      return;
   }

   CollisionLogEntry entry;
   entry.entity = data.collided_entity_ptr;
   entry.outcome = data.collision_outcome;
   entry.iteration = iteration;
   G_BUFFERS.collision_log->entries[size++] = entry;
}