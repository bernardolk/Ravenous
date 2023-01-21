// --------------
// PALETTE PANEL
// --------------

void render_palette_panel(PalettePanelContext* panel)
{
	ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
	ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
	ImGui::SetWindowSize("Palette", ImVec2(90, 500), ImGuiCond_Always);

	for(unsigned int i = 0; i < panel->count; i++)
	{
		if(ImGui::ImageButton((void*)static_cast<intptr_t>(panel->textures[i]), ImVec2(64, 64)))
		{
			auto attributes = panel->entity_palette[i];
			const auto new_entity = EntityManager.create_entity(attributes);
			new_entity->id = EntityManager.next_entity_id++;
			new_entity->position = GSceneInfo.camera->Position + (2.f * new_entity->scale + 5.f) * GSceneInfo.camera->Front;
			activate_move_mode(new_entity);
		}
	}

	ImGui::End();
}


void initialize_palette(PalettePanelContext* panel)
{
	int texture_count = 0;

	stbi_set_flip_vertically_on_load(false);
	panel->textures[texture_count++] = load_texture_from_file("box.png", EDITOR_ASSETS);
	panel->textures[texture_count++] = load_texture_from_file("slope.png", EDITOR_ASSETS);
	panel->textures[texture_count++] = load_texture_from_file("checkpoint.png", EDITOR_ASSETS);
	stbi_set_flip_vertically_on_load(true);

	// 0
	panel->entity_palette[panel->count++] = EntityAttributes{
	.name = "NONAME",
	.mesh = "aabb",
	.shader = "model",
	.texture = "grey",
	.collision_mesh = "aabb",
	.type = EntityType_Static
	};

	// 1
	panel->entity_palette[panel->count++] = EntityAttributes{
	.name = "NONAME",
	.mesh = "slope",
	.shader = "model",
	.texture = "grey",
	.collision_mesh = "slope",
	.type = EntityType_Static
	};

	// 3
	panel->entity_palette[panel->count++] = EntityAttributes{
	.name = "NONAME-CHECKPOINT",
	.mesh = "aabb",
	.shader = "model",
	.texture = "grey",
	.collision_mesh = "aabb",
	.type = EntityType_Checkpoint,
	.scale = vec3(0.3, 1.2, 0.3)
	};
}
