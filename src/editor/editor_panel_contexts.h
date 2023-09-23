#pragma once

#include "engine/core/core.h"
#include "EntityState.h"
#include "engine/catalogues.h"

namespace Editor
{
	struct InputRecorderPanelContext
	{
		bool active = false;
		int selected_recording = -1;
	};

	struct PalettePanelContext
	{
		bool active = true;
		unsigned int textures[15];
		EntityAttributes entity_palette[15];
		unsigned int count = 0;
	};

	struct SceneObjectsPanelContext
	{
		bool active = false;
		bool focused = false;
		std::string search_text = "";
	};

	struct EntityPanelContext
	{
		bool active = false;
		bool focused = false;
		E_Entity* entity = nullptr;
		vec3 original_position = vec3(0);
		vec3 original_scale = vec3(0);
		float original_rotation = 0;

		//rename buffer
		bool rename_option_active = false;
		const static u32 _rename_buff_size = 100;
		char rename_buffer[_rename_buff_size];

		bool reverse_scale = false;
		bool reverse_scale_x = false;
		bool reverse_scale_y = false;
		bool reverse_scale_z = false;

		E_Entity* x_arrow;
		E_Entity* y_arrow;
		E_Entity* z_arrow;

		E_Entity* rotation_gizmo_x;
		E_Entity* rotation_gizmo_y;
		E_Entity* rotation_gizmo_z;

		EntityState entity_starting_state;
		bool tracked_once = false;

		bool show_normals = false;
		bool show_collider = false;
		bool show_bounding_box = false;

		bool show_related_entity = false;
		E_Entity* related_entity = nullptr;

		void EmptyRenameBuffer()
		{
			for (int i = 0; i < _rename_buff_size; i++)
				rename_buffer[i] = 0;
		}

		bool ValidateRenameBufferContents()
		{
			for (int i = 0; i < _rename_buff_size; i++)
			{
				auto cursor = rename_buffer[i];
				if (cursor == '\0')
					return true;
				if (cursor == ' ' || cursor == 0)
					return false;
			}

			printf("Invalid c-string in rename buffer.\n");
			assert(false);
			return false;
		}
	};

	struct PlayerPanelContext
	{
		bool active = false;
		bool focused = false;
		Player* player;
	};

	struct WorldPanelContext
	{
		bool active = false;
		vec3 chunk_position_vec = vec3{-1.0f};
	};

	struct LightsPanelContext
	{
		bool active = false;
		bool focused = false;
		bool focus_tab = false;

		// selected light
		int selected_light = -1;
		float selected_light_yaw;
		float selected_light_pitch;
		std::string selected_light_type;
	};

	struct CollisionLogPanelContext
	{
		bool active = false;
		bool focused = false;
	};
}
