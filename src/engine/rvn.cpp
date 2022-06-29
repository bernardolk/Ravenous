#include <engine/rvn.h>

void RVN::init()
{
   rm_buffer      = new RenderMessageBuffer;
   entity_buffer  = new EntityBuffer;
}


void RVN::print_dynamic(const std::string& msg, float duration = 0, vec3 color = vec3(-1))
{
   rm_buffer->add(msg, duration, color);
}


void RVN::print(const std::string& msg, float duration = 0, vec3 color = vec3(-1))
{
   /*
      Will add a persistent message, that can't be updated later on, to the buffer.
   */
   rm_buffer->add_unique(msg, duration, color);
}


bool RenderMessageBuffer::add(const std::string& msg, float duration, vec3 color = vec3(-1))
{
   if(count >= size)
   {
      std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
      return false;
   }

   for(int i = 0; i < size; i++)
   {
      auto& item = buffer[i];
      // refresh message instead of adding if already exists
      if(item.message == msg)
      {
         item.elapsed = 0;
         break;
      }
      else if(item.message == "")
      {
         item.message     = msg;
         item.elapsed     = 0;
         item.duration    = duration;
         item.color       = color;
         count++;
         break;
      }
   }

   return true;
}


bool RenderMessageBuffer::add_unique(const std::string& msg, float duration, vec3 color = vec3(-1))
{
   if(count >= size)
   {
      std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
      return false;
   }

   for(int i = 0; i < size; i++)
   {
      auto& item = buffer[i];
      if(item.message == "")
      {
         item.message     = msg;
         item.elapsed     = 0;
         item.duration    = duration;
         item.color       = color;
         count++;
         break;
      }
   }

   return true;
}

void RenderMessageBuffer::cleanup()
{
   for(int i = 0; i < size; i++)
   {
      auto& item = buffer[i];
      item.elapsed += RVN::frame.duration * 1000.0;
      if(item.elapsed >= item.duration)
      {
         item.message = "";
         count -= 1;
      }
   }
}

void RenderMessageBuffer::render()
{
   int items_rendered = 0;
   for(int i = 0; i < size; i++)
   {
      auto& item = buffer[i];
      if(items_rendered == RVN::MAX_MESSAGES_TO_RENDER) break;

      if(item.message != "")
      {
         items_rendered++;
         render_text(
            "consola20",
            GlobalDisplayConfig::VIEWPORT_WIDTH / 2, 
            GlobalDisplayConfig::VIEWPORT_HEIGHT - 120 - items_rendered * 25,  
            item.color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : item.color,
            true,
            item.message
         );
      }
   }
}