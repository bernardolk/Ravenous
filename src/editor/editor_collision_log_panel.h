// --------------------
// COLLISION LOG PANEL
// --------------------

static void PushStyleCompact();
static void PopStyleCompact();

void render_collision_log_panel(CollisionLogPanelContext* panel)
{
   const u16 w_width  = 450;
   const u16 w_height = 320;
   const u8 w_bottom_margin = 30;
   const u8 table_items_to_show = 100;
   const u16 table_height = 120;

   ImGui::SetNextWindowPos(
      ImVec2(GlobalDisplayConfig::VIEWPORT_WIDTH - w_width, GlobalDisplayConfig::VIEWPORT_HEIGHT - w_height - w_bottom_margin), 
      ImGuiCond_Appearing
   );
   ImGui::Begin("Collision Log Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Collision Log Panel", ImVec2(w_width, w_height), ImGuiCond_Once);
   panel->focused = ImGui::IsWindowFocused();

   ImGuiTableFlags flags = 
      ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | 
      ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

   PushStyleCompact();
   ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
   PopStyleCompact();

   // When using ScrollX or ScrollY we need to specify a size for our table container!
   // Otherwise by default the table will fit all available space, like a BeginChild() call.
   ImVec2 outer_size = ImVec2(0.0f, table_height);
   if (ImGui::BeginTable("table_scrolly", 4, flags, outer_size))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn(".",              ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Entity",         ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Penetration",    ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Normal",         ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();

      ImGuiListClipper clipper;
      clipper.Begin(table_items_to_show);
      while (clipper.Step())
      {
         // for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
         // {
         //    CollisionLogEntry* entry = CL_read_collision_log_entry(row);
         //    if(entry == nullptr || entry->entity == NULL)
         //       break;
               
         //    ImGui::TableNextRow();
         //    ImGui::TableSetColumnIndex(0);
         //    ImGui::Text(std::to_string(row).c_str());
         //    ImGui::TableSetColumnIndex(1);
         //    ImGui::Text(entry->entity->name.c_str());
         //    ImGui::TableSetColumnIndex(2);
         //    ImGui::Text(std::to_string(entry->penetration).c_str());
         //    ImGui::TableSetColumnIndex(3);
         //    ImGui::Text(to_string(entry->normal).c_str());
         // }
      }
      ImGui::EndTable();
   }

   ImGui::End();
}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}



