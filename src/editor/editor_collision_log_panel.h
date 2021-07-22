// --------------------
// COLLISION LOG PANEL
// --------------------
static void PushStyleCompact();
static void PopStyleCompact();


void render_collision_log_panel(CollisionLogPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(180, 80), ImGuiCond_Appearing);
   ImGui::Begin("Collision Log Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Collision Log Panel", ImVec2(330, 900), ImGuiCond_Always);
   panel->focused = ImGui::IsWindowFocused();

   ImGuiTableFlags flags = 
      ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | 
      ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

   PushStyleCompact();
   ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
   PopStyleCompact();

   // When using ScrollX or ScrollY we need to specify a size for our table container!
   // Otherwise by default the table will fit all available space, like a BeginChild() call.
   ImVec2 outer_size = ImVec2(0.0f, 400);
   if (ImGui::BeginTable("table_scrolly", 3, flags, outer_size))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn(".",    ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Entity",    ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Outcome",   ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Iteration", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();

      ImGuiListClipper clipper;
      clipper.Begin(1000);
      while (clipper.Step())
      {
         for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
         {

            CollisionLogEntry* entry = read_collision_log_entry(row);
            if(entry == nullptr || entry->entity == NULL)
               break;
               
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(to_string(row).c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(entry->entity->name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(to_string(entry->outcome).c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text(to_string(entry->iteration).c_str());
         }
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



