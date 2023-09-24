#pragma once

#include "EditorInputRecorderPanel.h"
#include "EditorPanelContexts.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "tools/InputRecorder.h"
#include <imgui.h>

namespace Editor
{
	void RenderInputRecorderPanel(RInputRecorderPanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::viewport_width - 400, 800), ImGuiCond_Once);
		ImGui::Begin("Input Recorder Panel", &panel->active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("Input Recorder Panel", ImVec2(350, 220), ImGuiCond_Always);
		auto* input_recorder = RInputRecorder::Get();
		ImGui::Text("Recordings");
		for (int i = 0; i < input_recorder->recording_idx; i++)
		{
			std::string rec_name = "Recording #" + std::to_string(i);
			bool is_active = panel->selected_recording == i;
			if (ImGui::Checkbox(rec_name.c_str(), &is_active))
			{
				panel->selected_recording = i;
			}
		}

		if (input_recorder->recording_idx == 0)
		{
			ImGui::NewLine();
			ImGui::Text("No recordings yet.");
			ImGui::NewLine();
			ImGui::NewLine();
		}

		if (!(input_recorder->is_recording || input_recorder->is_playing))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
			if (ImGui::Button("Record", ImVec2(60, 18)))
			{
				Rvn::PrintDynamic("Input Recording Started", 2000);
				input_recorder->StartRecording();
			}
			ImGui::PopStyleColor(3);
		}

		if (input_recorder->is_recording)
		{
			if (ImGui::Button("Stop Recording", ImVec2(100, 18)))
			{
				Rvn::PrintDynamic("Input Recording Stoped", 2000);
				input_recorder->StopRecording();
			}
		}

		if (input_recorder->recording_idx > 0 && !(input_recorder->is_recording || input_recorder->is_playing))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.6f, 0.6f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.7f, 0.7f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.42f, 0.8f, 0.8f)));
			if (ImGui::Button("Play", ImVec2(60, 18)))
			{
				input_recorder->StartPlaying(panel->selected_recording);
			}
			ImGui::PopStyleColor(3);
		}

		if (input_recorder->is_playing)
		{
			if (ImGui::Button("Stop Playing", ImVec2(100, 18)))
			{
				input_recorder->StopPlaying();
			}
		}

		ImGui::End();
	}
}
