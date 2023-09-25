#include <string>
#include <iostream>
#include <engine/core/types.h>
#include <engine/render/text/TextRenderer.h>
#include <engine/rvn.h>
#include "engine/io/display.h"

ProgramConfig::ProgramConfig() = default;

void Rvn::Init()
{
	RmBuffer = new RenderMessageBuffer();
}


void Rvn::PrintDynamic(const std::string& Msg, float Duration, vec3 Color)
{
	RmBuffer->Add(Msg, Duration, Color);
}


void Rvn::Print(const std::string& Msg, float Duration, vec3 Color)
{
	/*
	   Will add a persistent Message, that can't be updated later on, to the buffer.
	*/
	RmBuffer->AddUnique(Msg, Duration, Color);
}


bool RenderMessageBuffer::Add(const std::string Msg, float Duration, vec3 Color)
{
	if (Count >= Capacity)
	{
		std::cout << "WARNING: Message has not been addded to Message buffer" << "because it is FULL. Message was: " << Msg << "\n";
		return false;
	}

	// @TODO: This is dumb :D
	for (int i = 0; i < Capacity; i++)
	{
		auto Item = &Buffer[i];
		// refresh Message instead of adding if already exists
		if (Item->Message == Msg)
		{
			Item->Elapsed = 0;
			break;
		}
		if (Item->Message == "")
		{
			new(Item) RenderMessageBufferElement{
			.Message = Msg,
			.Elapsed = 0,
			.Duration = Duration,
			.Color = Color
			};
			Count++;
			break;
		}
	}

	return true;
}


bool RenderMessageBuffer::AddUnique(const std::string Msg, float Duration, vec3 Color)
{
	if (Count >= Capacity)
	{
		std::cout << "WARNING: Message has not been addded to Message buffer" << "because it is FULL. Message was: " << Msg << "\n";
		return false;
	}

	// @TODO: This is dumb :D
	for (int i = 0; i < Capacity; i++)
	{
		auto Item = &Buffer[i];
		if (Item->Message == "")
		{
			new(Item) RenderMessageBufferElement{
			.Message = Msg,
			.Elapsed = 0,
			.Duration = Duration,
			.Color = Color
			};
			Count++;
			break;
		}
	}

	return true;
}

void RenderMessageBuffer::Cleanup()
{
	auto& Frame = RavenousEngine::GetFrame();

	for (int i = 0; i < Capacity; i++)
	{
		auto Item = &Buffer[i];
		Item->Elapsed += Frame.Duration * 1000.0;
		if (Item->Elapsed >= Item->Duration)
		{
			new(Item) RenderMessageBufferElement();
			Count -= 1;
		}
	}
}

void RenderMessageBuffer::Render()
{
	int ItemsRendered = 0;
	for (int i = 0; i < Capacity; i++)
	{
		auto& Item = Buffer[i];
		if (ItemsRendered == Rvn::MaxMessagesToRender)
			break;

		if (Item.Message != "")
		{
			ItemsRendered++;
			RenderText(
				"consola20",
				GlobalDisplayState::ViewportWidth / 2,
				GlobalDisplayState::ViewportHeight - 120 - ItemsRendered * 25,
				Item.Color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : Item.Color,
				true,
				Item.Message
			);
		}
	}
}
