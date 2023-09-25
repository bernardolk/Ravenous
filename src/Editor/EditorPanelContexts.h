#pragma once

#include "engine/core/core.h"
#include "EntityState.h"
#include "engine/catalogues.h"

namespace Editor
{
	struct RInputRecorderPanelContext
	{
		bool Active = false;
		int SelectedRecording = -1;
	};

	struct RPalettePanelContext
	{
		bool Active = true;
		unsigned int Textures[15];
		REntityAttributes EntityPalette[15];
		unsigned int Count = 0;
	};

	struct RSceneObjectsPanelContext
	{
		bool Active = false;
		bool Focused = false;
		string SearchText = "";
	};

	struct REntityPanelContext
	{
		bool Active = false;
		bool Focused = false;
		EEntity* Entity = nullptr;
		vec3 OriginalPosition = vec3(0);
		vec3 OriginalScale = vec3(0);
		float OriginalRotation = 0;

		//rename buffer
		bool RenameOptionActive = false;
		const static uint RenameBuffSize = 100;
		char RenameBuffer[RenameBuffSize];

		bool ReverseScale = false;
		bool ReverseScaleX = false;
		bool ReverseScaleY = false;
		bool ReverseScaleZ = false;

		EEntity* XArrow;
		EEntity* YArrow;
		EEntity* ZArrow;

		EEntity* RotationGizmoX;
		EEntity* RotationGizmoY;
		EEntity* RotationGizmoZ;

		REntityState EntityStartingState;
		bool TrackedOnce = false;

		bool ShowNormals = false;
		bool ShowCollider = false;
		bool ShowBoundingBox = false;

		bool ShowRelatedEntity = false;
		EEntity* RelatedEntity = nullptr;

		void EmptyRenameBuffer()
		{
			for (int I = 0; I < RenameBuffSize; I++)
				RenameBuffer[I] = 0;
		}

		bool ValidateRenameBufferContents()
		{
			for (int I = 0; I < RenameBuffSize; I++)
			{
				auto Cursor = RenameBuffer[I];
				if (Cursor == '\0')
					return true;
				if (Cursor == ' ' || Cursor == 0)
					return false;
			}

			printf("Invalid c-string in rename buffer.\n");
			assert(false);
			return false;
		}
	};

	struct RPlayerPanelContext
	{
		bool Active = false;
		bool Focused = false;
		EPlayer* Player = nullptr;
	};

	struct RWorldPanelContext
	{
		bool Active = false;
		vec3 ChunkPositionVec = vec3{-1.0f};
	};

	struct RLightsPanelContext
	{
		bool Active = false;
		bool Focused = false;
		bool FocusTab = false;

		// selected light
		int SelectedLight = -1;
		float SelectedLightYaw;
		float SelectedLightPitch;
		string SelectedLightType;
	};

	struct RCollisionLogPanelContext
	{
		bool Active = false;
		bool Focused = false;
	};
}
