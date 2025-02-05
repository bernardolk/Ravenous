#include "file_browser_modal.h"

#include <limits>
#include <dearIMGUI/imgui.h>
using namespace imgui_ext;

static void get_files_in_path(const fs::path& path, std::vector<file>& files, const char* filter) {
	files.clear();

	if (path.has_parent_path()) {
		files.push_back({
			"..",
			path.parent_path()
			});
	}

	for (const fs::directory_entry& dirEntry : fs::directory_iterator(path)) {
		const fs::path& dirPath = dirEntry.path();
		std::string filename = dirPath.filename().string();
		std::size_t index = filename.find_last_of(".");
		std::string extension = filename.substr(index + 1);
		if (extension == filter || filter == "" || index == std::string::npos) {
			files.push_back({
				dirPath.filename().string(),
				dirPath
				});
		}
	}
}

static const int clamp_size_t_to_int(const size_t data) {
	static const int max_int = std::numeric_limits<int>::max();
	return static_cast<int>(data > max_int ? max_int : data);
}

static bool vector_file_items_getter(void* data, int idx, const char** out_text) {
	const std::vector<file>* v = reinterpret_cast<std::vector<file>*>(data);
	const int elementCount = clamp_size_t_to_int(v->size());
	if (idx < 0 || idx >= elementCount) return false;
	*out_text = v->at(idx).alias.data();
	return true;
}

file_browser_modal::file_browser_modal(const char* title, const char* initial_path, const char* filter) :
	m_title(title),
	m_oldVisibility(false),
	m_selection(0),
	m_currentPath(fs::current_path()),
	m_currentPathIsDir(true),
	m_initialPath(initial_path),
	m_filter(filter){

}

// Will return true if file selected.
const bool file_browser_modal::render(const bool isVisible, std::string& outPath) {
	bool result = false;

	if (m_oldVisibility != isVisible) {
		m_oldVisibility = isVisible;
		//Visiblity has changed.

		if (isVisible) {
			//Only run when the visibility state changes to visible.

			//Reset the path to the initial path.
			//m_currentPath = fs::current_path();
			m_currentPath = m_initialPath;
			m_currentPathIsDir = true;

			//Update paths based on current path
			get_files_in_path(m_currentPath, m_filesInScope, m_filter);

			//Make the modal visible.
			ImGui::OpenPopup(m_title);
		}

	}

	bool isOpen = true;
	ImGuiWindowFlags modal_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_AlwaysAutoResize;

	if (ImGui::BeginPopupModal(m_title, &isOpen, modal_flags)) {

		if (ImGui::ListBox("##", &m_selection, vector_file_items_getter, &m_filesInScope, m_filesInScope.size(), 10)) {

			//Update current path to the selected list item.
			m_currentPath = m_filesInScope[m_selection].path;
			m_currentPathIsDir = fs::is_directory(m_currentPath);

			//If the selection is a directory, repopulate the list with the contents of that directory.
			if (m_currentPathIsDir) {
				get_files_in_path(m_currentPath, m_filesInScope, m_filter);
			}

		}

		//Auto resize text wrap to popup width.
		ImGui::PushItemWidth(-1);
		ImGui::TextWrapped(m_currentPath.string().data());
		ImGui::PopItemWidth();

		ImGui::Spacing();
		ImGui::SameLine(ImGui::GetWindowWidth() - 60);

		// Make the "Select" button look / act disabled if the current selection is a directory.
		if (m_currentPathIsDir) {

			static const ImVec4 disabledColor = { 0.3f, 0.3f, 0.3f, 1.0f };

			ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, disabledColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, disabledColor);

			ImGui::Button("Select");

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

		}
		else {

			if (ImGui::Button("Select")) {
				ImGui::CloseCurrentPopup();

				outPath = m_currentPath.string();
				result = true;
			}

		}

		ImGui::EndPopup();

	}

	return result;
}
