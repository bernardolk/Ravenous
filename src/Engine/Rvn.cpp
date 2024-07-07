#include <string>
#include <iostream>
#include <engine/core/types.h>
#include <engine/render/text/TextRenderer.h>
#include <engine/rvn.h>
#include "engine/io/display.h"

void Rvn::Init()
{
	EditorMsgManager = new REditorMsgManager();
}

void REditorMsgManager::AddMessage(uint& MsgId, const string MsgString, float Duration, vec3 Color)
{
	NextMsgDuration = DefaultMsgDuration;
	
	// refresh Message instead of adding if already exists
	for (auto& Msg : Messages) {
		if (Msg.Id == MsgId) {
			Msg.Elapsed = 0;
			return;
		}
	}

	// Adds new message
	MsgId = Hasher(MsgString);

	REditorMsg Msg;
	Msg.Color = Color;
	Msg.Duration = Duration * 1000;
	Msg.Id = MsgId;
	Msg.Message = MsgString;
	Messages.push_back(Msg);
}

void REditorMsgManager::Update()
{
	// Just update elapsed time of messages that are going to be render so we don't loose any msgs.
	auto& Frame = RavenousEngine::GetFrame();

	int ItemsToUpdate = MaxMessagesToRender;
	for (auto MsgIter = Messages.begin(); ItemsToUpdate == 0 || MsgIter != Messages.end();)
	{
		MsgIter->Elapsed += Frame.Duration * 1000.0;		
	
		if (MsgIter->Elapsed >= MsgIter->Duration) {
			MsgIter = Messages.erase(MsgIter);
		}

		--ItemsToUpdate;
	}
}

void REditorMsgManager::Render()
{
	int ItemsRendered = 0;
	for (auto& Msg : Messages)
	{
		if (ItemsRendered == MaxMessagesToRender) break;

		RenderText(
			"consola20",
			GlobalDisplayState::ViewportWidth / 2,
			GlobalDisplayState::ViewportHeight - 120 - ItemsRendered * 25,
			Msg.Color == vec3(-1) ? vec3(0.8, 0.8, 0.2) : Msg.Color,
			true,
			Msg.Message
		);

		ItemsRendered++;
	}
}
