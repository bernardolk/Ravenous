

struct ImmediateEditorValues {
   float val_float = 0.f;
   bool btn = false;
   bool btn2 = false;
} IM_Values;


#define _START_IM_ED() bool _active = true; \
   ImGui::SetNextWindowPos( \
      ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH / 2 - 200, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 100), ImGuiCond_Appearing \
   ); \
   ImGui::Begin("ImEdValues", &_active, ImGuiWindowFlags_AlwaysAutoResize);

#define _END_IM_ED() ImGui::End();


float IM_ED_float_slider(string label = "")
{
   _START_IM_ED();

   string _label = label == "" ? "Float value" : label;
   ImGui::InputFloat(_label.c_str(), &IM_Values.val_float);

   _END_IM_ED()

   return IM_Values.val_float;
}


bool IM_ED_toggle_btn(bool* btn, string label = "")
{
   _START_IM_ED();

   string _label = label == "" ? "Btn" : label;
   if(ImGui::Button(_label.c_str(), ImVec2(100,30)))
      *btn = !*btn;
   
   _END_IM_ED()

   return IM_Values.btn;
}
