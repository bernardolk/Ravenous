
#include <string>
#include <iostream>
#include <engine/core/rvn_types.h>
#include <map>
#include <engine/render/text/text_renderer.h>
#include <engine/rvn.h>

void RVN::init()
{
   rm_buffer      = new RenderMessageBuffer();
   entity_buffer  = new EntityBuffer();
}


void RVN::print_dynamic(const std::string& msg, float duration , vec3 color)
{
   rm_buffer->add(msg, duration, color);
}


void RVN::print(const std::string& msg, float duration, vec3 color)
{
   /*
      Will add a persistent message, that can't be updated later on, to the buffer.
   */
   rm_buffer->add_unique(msg, duration, color);
}


bool RenderMessageBuffer::add(const std::string msg, float duration, vec3 color)
{
   if(count >= capacity)
   {
      std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
      return false;
   }

   // @TODO: This is dumb :D
   for(int i = 0; i < capacity; i++)
   {
      auto item = &buffer[i];
      // refresh message instead of adding if already exists
      if(item->message == msg)
      {
         item->elapsed = 0;
         break;
      }
      else if(item->message == "")
      {
         new (item) RenderMessageBufferElement{
            .message     = msg,
            .elapsed     = 0,
            .duration    = duration,
            .color       = color
         };
         count++;
         break;
      }
   }

   return true;
}


bool RenderMessageBuffer::add_unique(const std::string msg, float duration, vec3 color)
{
   if(count >= capacity)
   {
      std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
      return false;
   }

   // @TODO: This is dumb :D
   for(int i = 0; i < capacity; i++)
   {
      auto item = &buffer[i];
      if(item->message == "")
      {
         new (item) RenderMessageBufferElement{
            .message     = msg,
            .elapsed     = 0,
            .duration    = duration,
            .color       = color
         };
         count++;
         break;
      }
   }

   return true;
}

void RenderMessageBuffer::cleanup()
{
   for(int i = 0; i < capacity; i++)
   {
      auto item = &buffer[i];
      item->elapsed += RVN::frame.duration * 1000.0;
      if(item->elapsed >= item->duration)
      {
         new (item) RenderMessageBufferElement();
         count -= 1;
      }
   }
}

void RenderMessageBuffer::render()
{
   int items_rendered = 0;
   for(int i = 0; i < capacity; i++)
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