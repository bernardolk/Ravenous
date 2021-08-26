// -----------
// CATALOGUES
// -----------
std::map<string, Mesh*> Geometry_Catalogue;
std::map<string, Shader*> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;

typedef std::map<GLchar, Character> gl_charmap;
std::map<string, gl_charmap> Font_Catalogue;

// ---------------------
// CAPACITY DEFINITIONS
// ---------------------
const size_t COLLISION_LOG_BUFFER_CAPACITY = 150;
const size_t COLLISION_LOG_CAPACITY = 20;
const size_t COLLISION_BUFFER_CAPACITY = WORLD_CELL_CAPACITY * 8;
const size_t MESSAGE_BUFFER_CAPACITY = 10;
const int MAX_MESSAGES_TO_RENDER = 8;

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

WorldStruct World;

struct GlobalSceneInfo {
   Scene* active_scene = NULL;
   Camera* camera;
   Camera* views[2];
   Player* player;
   bool input_mode = false;
   string scene_name;
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
   string message = "";
   float elapsed = 0;
   float duration = 0;
   vec3 color;
};

struct RenderMessageBuffer {
   RenderMessageBufferElement* buffer;
   size_t size;
   u16 count = 0;

   bool add(string msg, float duration, vec3 color = vec3(-1))
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
         cout << "WARNING: message has not been addded to message buffer"
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
void render_text(string font, float x, float y, vec3 color, bool center, string text);

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
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
            G_DISPLAY_INFO.VIEWPORT_HEIGHT - 120 - render_count * 25,  
            item->color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : item->color,
            true,
            item->message
         );
      }

      item++;
   }
}

// -----
// LOGS
// -----
// Stats and useful game data

// --------------
// collision log
// --------------
// Holds info about the collisions that recently happened
// Basically, it contains two buffers, a main and a swap one. Once the main is filled up, it starts writing in the swap,
// while still reading from the main buffer with an offset exactly equal to the write_count in the swap buffer, like
// a moving window. Once both are completely filled, we just swap the read and write buffers so we always start reading at
// the the tail and writing at the head with the same number of total entries. 

struct CollisionLogEntry {
   Entity* entity;
   CollisionOutcomeEnum outcome;
   int iteration;
};

struct CollisionLog {
   // buffers
   CollisionLogEntry* main;
   CollisionLogEntry* swap;
   int write_count = 0;
   // window
   CollisionLogEntry* read;
   CollisionLogEntry* write;
   const size_t window_size = COLLISION_LOG_CAPACITY;
};
CollisionLog* COLLISION_LOG;

void log_collision(CollisionData data, int iteration)
{    
   auto& log = COLLISION_LOG;
   if(log->write_count == COLLISION_LOG_CAPACITY)
   {
      // swap buffers
      if(log->read == log->write)
      {
         //log->move_window = true;
         log->write = log->swap;
      }
      else
      {
         auto temp =  log->read;
         log->read =  log->write;
         log->write = temp;
      }
      
      log->write_count = 0;
   }

   CollisionLogEntry entry;
   entry.entity = data.collided_entity_ptr;
   entry.outcome = data.collision_outcome;
   entry.iteration = iteration;
   log->write[log->write_count++] = entry;
}

CollisionLogEntry* read_collision_log_entry(int i)
{
   auto& log = COLLISION_LOG;

   // out of bounds check
   if(i >= COLLISION_LOG_CAPACITY)
      return nullptr;

   // we are not using swap buffers yet
   if(log->read == log->write)
      return log->read + i;

   int read_offset = log->write_count;
   // we need to read from the read buffer
   if(read_offset + i < COLLISION_LOG_CAPACITY)
      return log->read + (read_offset + i);

   // we need to read from the write buffer
   int write_offset = COLLISION_LOG_CAPACITY - read_offset;
   return log->write + (i - write_offset);
}

// -----------------------------
// BUFFERS AND LOGS ALLOCATIONS
// -----------------------------

EntityBuffer* allocate_entity_buffer()
{
   size_t size = COLLISION_BUFFER_CAPACITY;
   auto e_buffer = new EntityBuffer;
   e_buffer->buffer = new EntityBufferElement[size];
   e_buffer->size = size;
   return e_buffer;
}

RenderMessageBuffer* allocate_render_message_buffer()
{
   size_t size = MESSAGE_BUFFER_CAPACITY;
   auto rm_buffer = new RenderMessageBuffer;
   rm_buffer->buffer = new RenderMessageBufferElement[size];
   rm_buffer->size = size;
   return rm_buffer;
}

CollisionLog* allocate_collision_log()
{
   size_t size = COLLISION_LOG_CAPACITY;
   auto collision_log = new CollisionLog;
   collision_log->main = (CollisionLogEntry*) malloc(sizeof(CollisionLogEntry) * size * 2);
   collision_log->swap = collision_log->main + size;

   // initializes memory
   for(int i = 0; i < size * 2; i++)
      collision_log->main[i] = CollisionLogEntry{NULL};

   // set ptrs
   collision_log->read  = collision_log->main;
   collision_log->write = collision_log->main;

   return collision_log;
}

void RENDER_MESSAGE(string msg, float duration = 0, vec3 color = vec3(-1))
{
   G_BUFFERS.rm_buffer->add(msg, duration, color);
}