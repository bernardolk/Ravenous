#include <string>
#include <iostream>
#include <engine/core/types.h>
#include <map>
#include <engine/render/text/text_renderer.h>
#include <engine/rvn.h>
#include "engine/io/display.h"

void Rvn::Init()
{
	rm_buffer = new RenderMessageBuffer();
	entity_buffer = new EntityBuffer();
}


void Rvn::PrintDynamic(const std::string& msg, float duration, vec3 color)
{
	rm_buffer->Add(msg, duration, color);
}


void Rvn::Print(const std::string& msg, float duration, vec3 color)
{
	/*
	   Will add a persistent message, that can't be updated later on, to the buffer.
	*/
	rm_buffer->AddUnique(msg, duration, color);
}


bool RenderMessageBuffer::Add(const std::string msg, float duration, vec3 color)
{
	if (count >= capacity)
	{
		std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
		return false;
	}

	// @TODO: This is dumb :D
	for (int i = 0; i < capacity; i++)
	{
		auto item = &buffer[i];
		// refresh message instead of adding if already exists
		if (item->message == msg)
		{
			item->elapsed = 0;
			break;
		}
		if (item->message == "")
		{
			new(item) RenderMessageBufferElement{
			.message = msg,
			.elapsed = 0,
			.duration = duration,
			.color = color
			};
			count++;
			break;
		}
	}

	return true;
}


bool RenderMessageBuffer::AddUnique(const std::string msg, float duration, vec3 color)
{
	if (count >= capacity)
	{
		std::cout << "WARNING: message has not been addded to message buffer" << "because it is FULL. Message was: " << msg << "\n";
		return false;
	}

	// @TODO: This is dumb :D
	for (int i = 0; i < capacity; i++)
	{
		auto item = &buffer[i];
		if (item->message == "")
		{
			new(item) RenderMessageBufferElement{
			.message = msg,
			.elapsed = 0,
			.duration = duration,
			.color = color
			};
			count++;
			break;
		}
	}

	return true;
}

void RenderMessageBuffer::Cleanup()
{
	for (int i = 0; i < capacity; i++)
	{
		auto item = &buffer[i];
		item->elapsed += Rvn::frame.duration * 1000.0;
		if (item->elapsed >= item->duration)
		{
			new(item) RenderMessageBufferElement();
			count -= 1;
		}
	}
}

void RenderMessageBuffer::Render()
{
	int items_rendered = 0;
	for (int i = 0; i < capacity; i++)
	{
		auto& item = buffer[i];
		if (items_rendered == Rvn::max_messages_to_render)
			break;

		if (item.message != "")
		{
			items_rendered++;
			render_text(
				"consola20",
				GlobalDisplayConfig::viewport_width / 2,
				GlobalDisplayConfig::viewport_height - 120 - items_rendered * 25,
				item.color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : item.color,
				true,
				item.message
			);
		}
	}
}
