

struct IM_ED_values {
   float val_float = 0.f;
} IM_ED_Values;


float IM_ED_float_slider(string label = "")
{
   bool _im_ed_float_slider_active = true; 
   ImGui::SetNextWindowPos(
      ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH / 2 - 200, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 100), ImGuiCond_Appearing
   );
   string _label = label == "" ? "IM FLOAT" : label;
   ImGui::Begin(_label.c_str(), &_im_ed_float_slider_active, ImGuiWindowFlags_AlwaysAutoResize);
   ImGui::InputFloat("float value", &IM_ED_Values.val_float);
   ImGui::End();

   return IM_ED_Values.val_float;
}
      