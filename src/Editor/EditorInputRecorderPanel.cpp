#pragma once

#include "EditorInputRecorderPanel.h"
#include "EditorPanelContexts.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "tools/InputRecorder.h"
#include <imgui.h>

namespace Editor
{
	void RenderInputRecorderPanel(RInputRecorderPanelContext* Panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::ViewportWidth - 400, 800), ImGuiCond_Once);
		ImGui::Begin("Input Recorder Panel", &Panel->Active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("Input Recorder Panel", ImVec2(350, 220), ImGuiCond_Always);
		auto* InputRecorder = RInputRecorder::Get();
		ImGui::Text("Recordings");
		for (int i = 0; i < InputRecorder->RecordingIdx; i++)
		{
			std::string RecName = "Recording #" + std::to_string(i);
			bool IsActive = Panel->SelectedRecording == i;
			if (ImGui::Checkbox(RecName.c_str(), &IsActive))
			{
				Panel->SelectedRecording = i;
			}
		}

		if (InputRecorder->RecordingIdx == 0)
		{
			ImGui::NewLine();
			ImGui::Text("No recordings yet.");
			ImGui::NewLine();
			ImGui::NewLine();
		}

		if (!(InputRecorder->bIsRecording || InputRecorder->bIsPlaying))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
			if (ImGui::Button("Record", ImVec2(60, 18)))
			{
				//Rvn::PrintDynamic("Input Recording Started", 2000);
				InputRecorder->StartRecording();
			}
			ImGui::PopStyleColor(3);
		}

		if (InputRecorder->bIsRecording)
		{
			if (ImGui::Button("Stop Recording", ImVec2(100, 18)))
			{
				//Rvn::PrintDynamic("Input Recording Stoped", 2000);
				InputRecorder->StopRecording();
			}
		}

		if (InputRecorder->RecordingIdx > 0 && !(InputRecorder->bIsRecording || InputRecorder->bIsPlaying))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Play", ImVec2(60, 18)))
			{
				InputRecorder->StartPlaying(Panel->SelectedRecording);
			}
			ImGui::PopStyleColor(3);
		}

		if (InputRecorder->bIsPlaying)
		{
			if (ImGui::Button("Stop Playing", ImVec2(100, 18)))
			{
				InputRecorder->StopPlaying();
			}
		}

		ImGui::End();
	}
}
