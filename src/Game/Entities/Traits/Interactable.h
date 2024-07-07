#pragma once
#include "engine/core/core.h"
#include "engine/entities/traits/EntityTraits.h"
#include "Engine/Geometry/Cylinder.h"
#include "Engine/Render/ImRender.h"
#include "engine/rvn.h"

/*  ===========================================================================================================
 *	Interactable
 *		Trait that allows entities to be interacted with via action key or touching their trigger volume.
 *	=========================================================================================================== */

struct Trait(TInteractable)
{
	Reflected_Trait(TInteractable)

	Field(bool, bBlockInteraction) = false;
	Field(bool, bPassiveInteraction) = false;
	Field(bool, bInteractOnlyWhenLookingAtEntity) = true;

   /* ========================================
	* Update
	* ======================================== */	
	template<typename T>
	static void Update(T& Entity)
	{
		auto Centroid = Entity.BoundingBox.GetCentroid();
		Entity.Cylinder.Position = {Centroid.x, Entity.Position.y, Centroid.z};
		static RMesh** Cylinder = Find(GeometryCatalogue, "cylinder");
		if (Cylinder) {
			RImDraw::AddMeshWithTransform(IMHASH, *Cylinder, Entity.Cylinder.Position, vec3{0.f}, vec3{Entity.Cylinder.Radius, Entity.Cylinder.Height, Entity.Cylinder.Radius});
		}

		SetEditorMsgDuration(0.f);
		if (!Entity.IsPlayerLookingAtEntity(&Entity)) {
			PrintEditorMsg("No");
		}
		else {
			PrintEditorMsg("Yes");
			Draw();
		}
		if (!Entity.bBlockInteraction) {
			if (Entity.bPassiveInteraction || Entity.IsPlayerInteracting()) {
				if (Entity.IsVolumeCollidingWithPlayer()) {
					if (!Entity.bInteractOnlyWhenLookingAtEntity || Entity.IsPlayerLookingAtEntity(&Entity)) {
						Entity.Interact();
					}
				}
			}
		}
	}

protected:
	Field(RCylinder, Cylinder);

private:
	bool IsVolumeCollidingWithPlayer();
	bool IsPlayerLookingAtEntity(EEntity* Entity);
	bool IsPlayerInteracting();
	static void Draw();
};
