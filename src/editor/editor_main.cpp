#include "editor_main.h"

void Editor::start_dear_imgui_frame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void Editor::end_dear_imgui_frame()
{
	ImGui::EndFrame();
}