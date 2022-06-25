// ---------------------
// CAPACITY DEFINITIONS
// ---------------------
const size_t   COLLISION_LOG_BUFFER_CAPACITY = 150;
const size_t   COLLISION_LOG_CAPACITY = 20;
const size_t   COLLISION_BUFFER_CAPACITY = WORLD_CELL_CAPACITY * 8;
const size_t   MESSAGE_BUFFER_CAPACITY = 10;
const int      MAX_MESSAGES_TO_RENDER = 8;

// ----------------------
// global buffers struct
// ----------------------
// holds all buffers

struct EntityBuffer;
struct RenderMessageBuffer;

struct GlobalBuffers {
   EntityBuffer* entity_buffer;
   RenderMessageBuffer* rm_buffer;
} G_BUFFERS;

// -----------------
//  WORLD AND SCENE
// -----------------
// spatial partitioning struct and scene data

struct GlobalSceneInfo {
   Scene* active_scene = NULL;
   Camera* camera;
   Camera* views[2];
   Player* player;
   bool input_mode = false;
  std::string scene_name;

   bool tmp_unstuck_things = false;
} G_SCENE_INFO;

// ---------------
// GLOBAL BUFFERS
// ---------------
// here we define buffers that hold data for a brief ammount of time for multiple purposes

// --------------
// entity buffer
// --------------
// stores all relevant entity ptrs for collision detection with player during the frame

struct EntityBufferElement {
   Entity* entity;
   bool collision_check = false;
};

struct EntityBuffer {
   EntityBufferElement* buffer;
   int size;
};

// ----------------------
// render message buffer
// ----------------------
// stores messages to be displayed on screen during a certain duration

struct RenderMessageBufferElement {
  std::string message = "";
   float elapsed = 0;
   float duration = 0;
   vec3 color;
};

struct RenderMessageBuffer {
   RenderMessageBufferElement* buffer;
   size_t size;
   u16 count = 0;

   bool add(std::string msg, float duration, vec3 color = vec3(-1))
   {
      if(count < size)
      {
         auto item = buffer;
         for(int i = 0; i < size; i++)
         {
            // refresh message instead of adding if already exists
            if(item->message == msg)
            {
               item->elapsed = 0;
               break;
            }
            else if(item->message == "")
            {
               item->message = msg;
               item->elapsed = 0;
               item->duration = duration;
               item->color = color;
               count++;
               break;
            }
            item++;
         }
         return true;
      }
      else
      {
         std::cout << "WARNING: message has not been addded to message buffer"
         << "because it is FULL. Message was: " << msg << "\n";
         return false;
      }
   }

   bool add_unique(std::string msg, float duration, vec3 color = vec3(-1))
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
               item->color = color;
               count++;
               break;
            }
            item++;
         }
         return true;
      }
      else
      {
         std::cout << "WARNING: message has not been addded to message buffer"
         << "because it is FULL. Message was: " << msg << "\n";
         return false;
      }
   }
};

void expire_render_messages_from_buffer()
{
   size_t size = G_BUFFERS.rm_buffer->size;
   auto item = G_BUFFERS.rm_buffer->buffer;
   for(int i = 0; i < size; i++)
   {
      item->elapsed += G_FRAME_INFO.duration * 1000.0;
      if(item->message != "" && item->elapsed >= item->duration)
      {
         item->message = "";
         G_BUFFERS.rm_buffer->count -= 1;
      }
      item++;
   }
}

// fw decl.
void render_text(std::string font, float x, float y, vec3 color, bool center,std::string text);

void render_message_buffer_contents()
{
   //@todo make disappearing effect
   int render_count = 0;
   size_t size = G_BUFFERS.rm_buffer->size;
   auto item = G_BUFFERS.rm_buffer->buffer;
   for(int i = 0; i < size; i++)
   {
      if(render_count == MAX_MESSAGES_TO_RENDER)
         break;

      if(item->message != "")
      {
         render_count++;
         render_text(
            "consola20",
            GlobalDisplayConfig::VIEWPORT_WIDTH / 2, 
            GlobalDisplayConfig::VIEWPORT_HEIGHT - 120 - render_count * 25,  
            item->color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : item->color,
            true,
            item->message
         );
      }

      item++;
   }
}

// -------------------------------
// > BUFFERS AND LOGS ALLOCATIONS
// -------------------------------

EntityBuffer* allocate_entity_buffer()
{
   size_t size          = COLLISION_BUFFER_CAPACITY;
   auto e_buffer        = new EntityBuffer;
   e_buffer->buffer     = new EntityBufferElement[size];
   e_buffer->size       = size;
   return e_buffer;
}


RenderMessageBuffer* allocate_render_message_buffer()
{
   size_t size          = MESSAGE_BUFFER_CAPACITY;
   auto rm_buffer       = new RenderMessageBuffer;
   rm_buffer->buffer    = new RenderMessageBufferElement[size];
   rm_buffer->size      = size;
   return rm_buffer; 
}


void editor_print(std::string msg, float duration = 0, vec3 color = vec3(-1))
{
   G_BUFFERS.rm_buffer->add(msg, duration, color);
}

void editor_persist_print(std::string msg, float duration = 0, vec3 color = vec3(-1))
{
   // same as above, but in this case we don't update the message if already present 
   // (good for counting num of times for things that happen not that many times)
   G_BUFFERS.rm_buffer->add_unique(msg, duration, color);
}