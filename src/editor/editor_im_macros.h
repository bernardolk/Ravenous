#pragma once

/*
   Here you find values and components that can be used with dear ImGui for quick interactivity 
   in any part of the program, as long as you are using Editor mode
*/

inline struct ImmediateEditorValues
{
	float val_float = 0.f;
	int val_int = 0;
	bool btn = false;
	bool btn2 = false;
} ImValues;


#define _START_IM_ED() bool _active = true; \
   if(ProgramMode.current == EDITOR_MODE) { \
   ImGui::SetNextWindowPos( \
      ImVec2(GlobalDisplayConfig::viewport_width / 2 - 200, GlobalDisplayConfig::viewport_height - 100), ImGuiCond_Appearing \
   ); \
   ImGui::Begin("ImEdValues", &_active, ImGuiWindowFlags_AlwaysAutoResize);

#define _END_IM_ED() ImGui::End(); };


inline float IM_ED_float_slider(std::string label = "")
{
	_START_IM_ED();

		std::string _label = label == "" ? "Float value" : label;
		ImGui::InputFloat(_label.c_str(), &ImValues.val_float);

		_END_IM_ED()

	return ImValues.val_float;
}


inline bool IM_ED_toggle_btn(bool* btn, std::string label = "")
{
	_START_IM_ED();

		std::string _label = label == "" ? "Btn" : label;
		if(ImGui::Button(_label.c_str(), ImVec2(100, 30)))
			*btn = !*btn;

		_END_IM_ED()

	return ImValues.btn;
}
