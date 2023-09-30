#pragma once

#include "EditorTools.h"
#include "editor/editor.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/collision/ClController.h"
#include "engine/io/input.h"
#include "engine/world/World.h"
#include "game/entities/EPlayer.h"

namespace Editor
{
	void DeactivateEditorModes()
	{
		auto& EdContext = *GetContext();

		EdContext.MoveMode = false;
		EdContext.SnapMode = false;
		EdContext.MeasureMode = false;
		EdContext.FirstPointFound = false;
		EdContext.SecondPointFound = false;
		EdContext.StretchMode = false;
		EdContext.LocateCoordsMode = false;
		EdContext.PlaceMode = false;
		EdContext.SelectEntityAuxMode = false;
	}

	bool CheckModesAreActive()
	{
		auto& EdContext = *GetContext();

		return
		EdContext.MoveMode ||
		EdContext.SnapMode ||
		EdContext.MeasureMode ||
		EdContext.StretchMode ||
		EdContext.LocateCoordsMode;
	}

	void EditorEraseEntity(EEntity* Entity)
	{
		//TODO: To be implemented
		auto& EdContext = *GetContext();

		EdContext.UndoStack.DeletionLog.Add(Entity);
	}

	void EditorEraseLight(int Index, string Type, RWorld* World)
	{
		auto& EdContext = *GetContext();

		if (Type == "point")
		{
			World->PointLights.erase(World->PointLights.begin() + Index);
		}
		else if (Type == "spot")
		{
			World->SpotLights.erase(World->SpotLights.begin() + Index);
		}

		if (EdContext.LightsPanel.SelectedLight == Index)
			EdContext.LightsPanel.SelectedLight = -1;
	}

	void UnhideEntities(RWorld* World)
	{
		auto EntityIter = World->GetEntityIterator();
		while (auto* Entity = EntityIter())
		{
			if (Entity->Flags & EntityFlags_HiddenEntity)
				Entity->Flags &= ~EntityFlags_HiddenEntity;
		}
	}

	// ----------
	// SNAP TOOL
	// ----------
	void ActivateSnapMode(EEntity* Entity);
	void SnapEntityToReference(EEntity* Entity);
	void CheckSelectionToSnap();
	void SnapCommit();

	void ActivateSnapMode(EEntity* Entity)
	{
		// deactivate_editor_modes();
		// EdContext.SnapMode = true;
		// EdContext.SnapTrackedState = get_entity_state(entity);
		// EdContext.UndoStack.track(entity);
	}

	void SnapCommit()
	{
		// auto entity = EdContext.EntityPanel.entity;
		// EdContext.SnapTrackedState = get_entity_state(entity);
		// EdContext.UndoStack.track(entity);
	}

	void SnapEntityToReference(EEntity* Entity)
	{
		// auto reference = EdContext.SnapReference;
		// float diff = 0;
		// auto diff_vec = vec3(0.0f);
		// auto [x0, x1, z0, z1]         = reference->get_rect_bounds();
		// auto [e_x0, e_x1, e_z0, e_z1] = Entity->get_rect_bounds();

		// switch(EdContext.SnapAxis)
		// {
		//    case 0:  // x
		//       if     (EdContext.SnapCycle == 0 && !EdContext.SnapInside) diff_vec.x = x1 - e_x0;
		//       else if(EdContext.SnapCycle == 0 &&  EdContext.SnapInside) diff_vec.x = x1 - e_x0 - (e_x1 - e_x0);
		//       else if(EdContext.SnapCycle == 2 && !EdContext.SnapInside) diff_vec.x = x0 - e_x1;
		//       else if(EdContext.SnapCycle == 2 &&  EdContext.SnapInside) diff_vec.x = x0 - e_x1 + (e_x1 - e_x0);
		//       else if(EdContext.SnapCycle == 1 ) diff_vec.x = x1 - e_x1 - (x1 - x0) / 2.0 + (e_x1 - e_x0) / 2.0;
		//       break;
		//    case 1:  // y
		//    {
		//       float bottom = reference->Position.y;
		//       float height = reference->get_height();
		//       float top = bottom + height;
		//       float current_bottom = Entity->Position.y;
		//       float current_top = current_bottom + Entity->get_height();

		//       if     (EdContext.SnapCycle == 0 && !EdContext.SnapInside) diff_vec.y = top - current_bottom;
		//       else if(EdContext.SnapCycle == 0 &&  EdContext.SnapInside) diff_vec.y = top - current_top;
		//       else if(EdContext.SnapCycle == 2 && !EdContext.SnapInside) diff_vec.y = bottom - current_top;
		//       else if(EdContext.SnapCycle == 2 &&  EdContext.SnapInside) diff_vec.y = bottom - current_bottom;
		//       else if(EdContext.SnapCycle == 1 && !EdContext.SnapInside) diff_vec.y = top - height / 2.0 - current_top;
		//       break;
		//    }
		//    case 2: // z
		//       if     (EdContext.SnapCycle == 0 && !EdContext.SnapInside) diff_vec.z = z1 - e_z0;
		//       else if(EdContext.SnapCycle == 0 &&  EdContext.SnapInside) diff_vec.z = z1 - e_z0 - (e_z1 - e_z0);
		//       else if(EdContext.SnapCycle == 2 && !EdContext.SnapInside) diff_vec.z = z0 - e_z1;
		//       else if(EdContext.SnapCycle == 2 &&  EdContext.SnapInside) diff_vec.z = z0 - e_z1 + (e_z1 - e_z0);
		//       else if(EdContext.SnapCycle == 1 ) diff_vec.z = z1 - e_z1 - (z1 - z0) / 2.0 + (e_z1 - e_z0) / 2.0;
		//       break;
		// }

		// Entity->Position += diff_vec;
	}


	void CheckSelectionToSnap()
	{
		// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.MouseCoords.x, G_INPUT_INFO.MouseCoords.y);
		// auto test = test_ray_against_scene(pickray);
		// if(test.hit)
		// {
		//    EdContext.SnapReference = test.entity;
		//    snap_entity_to_reference(EdContext.EntityPanel.entity);
		// }
	}

	// -------------
	// STRETCH TOOL
	// -------------
	void ActivateStretchMode(EEntity* Entity);
	void StretchCommit();
	auto GetScaleAndPositionChange(EEntity* Entity, float OldPos, float NewPos, float N);
	void StretchEntityToReference(EEntity* Entity);
	void CheckSelectionToStretch(REntityPanelContext* Panel);

	void ActivateStretchMode(EEntity* Entity)
	{
		// deactivate_editor_modes();
		// EdContext.StretchMode = true;
		// EdContext.UndoStack.track(entity);
	}

	void StretchCommit()
	{
		// auto entity = EdContext.EntityPanel.entity;
		// EdContext.UndoStack.track(entity);
	}

	auto GetScaleAndPositionChange(float EScale, float EAligned, float EOpposite, float T, float N)
	{
		/* e_aligned -> entity mesh triangle pos in axis with normal aligned with selected t triangle normal
		   e_opposite -> entity mesh triangle pos in axis with normal in same direction but opposite sense with selected t triangle normal
		   e_scale -> entity scale in axis
		   t -> pos of selected triangle in axis
		   n -> axis component of t normal (sign) */

		struct
		{
			float scale_f;
			float pos_f = 0.f;
		} Transform;

		float Dif = EAligned - T;

		bool Shrink = (Dif > 0 && N > 0) || (Dif < 0 && N < 0);

		// if we are going to invert the scale of the entity by shrinking it towards itself,
		// then we stretch using the other side of the entity to achieve expected behaviour.
		if (Shrink && abs(Dif) >= EScale)
		{
			Dif = EOpposite - T;
			N *= -1;
			Shrink = false;
		}

		if (Shrink)
			Transform.scale_f = -1.0 * abs(Dif);
		else
			Transform.scale_f = abs(Dif);

		// if normal points to negative dir, move Position
		if (N < 0)
			Transform.pos_f -= Dif;

		return Transform;
	}

	void StretchEntityToReference(EEntity* Entity, RTriangle T)
	{
		// // In this function, we are, obviously, considering that
		// // the triangle is axis aligned
		// assert(is_valid(t));

		// vec3 normal = glm::triangleNormal(t.a, t.b, t.c);

		// // assert triangles are axis aligned always
		// if(abs(normal.x) != 1 && abs(normal.y) != 1 && abs(normal.z) != 1) 
		// {
		//    RVN::rm_buffer->add("Stretch failed", 1200);
		//    return;
		// }

		// // test each triangle from entity mesh until finding face
		// // which normal is equivalent to the reference triangle
		// // normal vector
		// // Note: we find 2 triangles per normal thats why we have the bools
		// Triangle e_aligned, e_opposite;
		// bool aligned = false;
		// bool opposite = false;
		// int triangles = Entity->mesh->indices.size() / 3;
		// for(int i = 0; i < triangles; i++)
		// {
		//    Triangle _t = get_triangle_for_indexed_mesh(Entity->mesh, Entity->matModel, i);
		//    vec3 _normal = glm::triangleNormal(_t.a, _t.b, _t.c);

		//    if(is_equal(normal, _normal) && !aligned)
		//    {
		//       e_aligned = _t;
		//       aligned = true;
		//    }
		//    else if(is_equal(normal, -1.f * _normal) && !opposite)
		//    {
		//       e_opposite = _t;
		//       opposite = true;
		//    }

		//    if(aligned && opposite)
		//       break;
		// }

		// // if this breaks, check normal comparisons in loop above or if entities have right-hand convention
		// // in vertices definitions in geometry
		// assert(aligned && opposite);

		// // note: since triangles are axis aligned, it doesn't matter from
		// // which vertice (a, b, c) we take the coordinates.
		// int axis_count = 0;
		// if(!is_zero(normal.x))
		// {
		//    auto [s, p] = get_scale_and_Position_change(Entity->scale.x, e_aligned.a.x, e_opposite.a.x, t.a.x, normal.x);
		//    Entity->scale.x += s;
		//    Entity->Position.x += p;
		//    axis_count++;
		// }
		// if(!is_zero(normal.y))
		// {
		//    auto [s, p] = get_scale_and_Position_change(Entity->scale.y, e_aligned.a.y, e_opposite.a.y, t.a.y, normal.y);
		//    Entity->scale.y += s;
		//    Entity->Position.y += p;
		//    axis_count++;
		// }
		// if(!is_zero(normal.z))
		// {
		//    auto [s, p] = get_scale_and_Position_change(Entity->scale.z, e_aligned.a.z, e_opposite.a.z, t.a.z, normal.z);
		//    Entity->scale.z += s;
		//    Entity->Position.z += p;
		//    axis_count++;
		// }

		// assert(axis_count == 1);
		// return;
	}



	void CheckSelectionToStretch()
	{
		// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.MouseCoords.x, G_INPUT_INFO.MouseCoords.y);
		// auto test = test_ray_against_scene(pickray);
		// if(test.hit)
		// {
		//    stretch_entity_to_reference(EdContext.EntityPanel.entity, test.t);
		//    stretch_commit();
		// }
	}


	// -------------
	// MEASURE TOOL
	// -------------
	void ActivateMeasureMode(uint8 Axis);
	void CheckSelectionToMeasure(const RWorld* World);

	void ActivateMeasureMode(uint8 Axis)
	{
		auto& EdContext = *GetContext();

		DeactivateEditorModes();
		EdContext.MeasureMode = true;
		EdContext.MeasureAxis = Axis;
	}

	void CheckSelectionToMeasure(const RWorld* World)
	{
		auto* GII = GlobalInputInfo::Get();
		auto* CamManager = RCameraManager::Get();
		auto& EdContext = *GetContext();

		auto Pickray = CastPickray(CamManager->GetCurrentCamera(), GII->MouseCoords.X, GII->MouseCoords.Y);
		auto Test = World->Raycast(Pickray);
		if (Test.Hit)
		{
			if (!EdContext.FirstPointFound || EdContext.SecondPointFound)
			{
				if (EdContext.SecondPointFound)
					EdContext.SecondPointFound = false;
				EdContext.FirstPointFound = true;
				EdContext.MeasureFrom = ClGetPointFromDetection(Pickray, Test);
			}
			else if (!EdContext.SecondPointFound)
			{
				EdContext.SecondPointFound = true;
				vec3 Point = ClGetPointFromDetection(Pickray, Test);
				if (EdContext.MeasureAxis == 0)
					EdContext.MeasureTo = Point.x;
				else if (EdContext.MeasureAxis == 1)
					EdContext.MeasureTo = Point.y;
				else if (EdContext.MeasureAxis == 2)
					EdContext.MeasureTo = Point.z;
			}
		}
	}

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void ActivateLocateCoordsMode();
	void CheckSelectionToLocateCoords(const RWorld* World);

	void ActivateLocateCoordsMode()
	{
		auto& EdContext = *GetContext();

		DeactivateEditorModes();
		EdContext.LocateCoordsMode = true;
		EdContext.LocateCoordsFoundPoint = false;
	}

	void CheckSelectionToLocateCoords(const RWorld* World)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& EdContext = *GetContext();

		auto Pickray = CastPickray(RCameraManager::Get()->GetCurrentCamera(), GII->MouseCoords.X, GII->MouseCoords.Y);
		auto Test = World->Raycast(Pickray);
		if (Test.Hit)
		{
			EdContext.LocateCoordsFoundPoint = true;
			EdContext.LocateCoordsPosition = ClGetPointFromDetection(Pickray, Test);
		}
	}

	// -------------
	// > MOVE TOOLS 
	// -------------
	void PlaceEntity(RWorld* World)
	{
		/* Common function for move/rotate/scale entity tools.
		   Updates entity, tracks it state and updates world.
		   To be called at the end of entity modification operation. */
		auto& EdContext = *GetContext();

		EdContext.MoveMode = false;
		EdContext.MoveEntityByArrows = false;
		EdContext.RotateEntityWithMouse = false;
		EdContext.PlaceMode = false;
		EdContext.MoveEntityByArrowsRefPoint = vec3(0);

		EdContext.SelectedEntity->Update();
		World->UpdateEntityWorldChunk(EdContext.SelectedEntity);
		ClRecomputeCollisionBufferEntities();
		EdContext.UndoStack.Track(EdContext.SelectedEntity);
	}

	RRaycastTest TestRayAgainstEntitySupportPlane(uint16 MoveAxis, EEntity* Entity)
	{
		// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
		// Position. In the case of Y placement, we need to compute the plane considering the camera orientation.
		RTriangle T1, T2;
		float PlaneSize = 500.0f;
		auto& EdContext = *GetContext();
		auto* Camera = RCameraManager::Get()->GetCurrentCamera();

		switch (EdContext.MoveAxis)
		{
			case 0: // XZ 
			case 1: // X
			case 3: // Z
				T1.A = vec3{Entity->Position.x - PlaneSize, Entity->Position.y, Entity->Position.z - PlaneSize};
				T1.B = vec3{Entity->Position.x + PlaneSize, Entity->Position.y, Entity->Position.z - PlaneSize};
				T1.C = vec3{Entity->Position.x + PlaneSize, Entity->Position.y, Entity->Position.z + PlaneSize};
				T2.A = vec3{Entity->Position.x - PlaneSize, Entity->Position.y, Entity->Position.z - PlaneSize};
				T2.B = vec3{Entity->Position.x - PlaneSize, Entity->Position.y, Entity->Position.z + PlaneSize};
				T2.C = vec3{Entity->Position.x + PlaneSize, Entity->Position.y, Entity->Position.z + PlaneSize};
				break;
			case 2: // Y
			{
				// creates vector from cam to entity in XZ
				vec3 CamToEntity = Camera->Position - Entity->Position;
				CamToEntity.y = Camera->Position.y;
				CamToEntity = normalize(CamToEntity);
				// finds vector that lie in plane considering cam to entity vector as normal to it
				vec3 UpVec = normalize(vec3{Camera->Position.x, 1.0f, Camera->Position.z});
				vec3 VecInPlane = glm::cross(UpVec, CamToEntity);

				// creates plane
				T1.A = Entity->Position + (VecInPlane * -1.0f * PlaneSize);
				T1.A.y = Camera->Position.y + -1.0f * PlaneSize;

				T1.B = Entity->Position + (VecInPlane * PlaneSize);
				T1.B.y = Camera->Position.y + -1.0f * PlaneSize;

				T1.C = Entity->Position + (VecInPlane * PlaneSize);
				T1.C.y = Camera->Position.y + PlaneSize;

				T2.A = T1.A;
				T2.B = Entity->Position + (VecInPlane * -1.0f * PlaneSize);
				T2.B.y = Camera->Position.y + PlaneSize;
				T2.C = T1.C;

				break;
			}
		}

		// ray casts against created plane
		auto* GII = GlobalInputInfo::Get();

		auto Ray = CastPickray(Camera, GII->MouseCoords.X, GII->MouseCoords.Y);
		RRaycastTest Test;

		Test = ClTestAgainstRay(Ray, T1);
		if (!Test.Hit)
		{
			Test = ClTestAgainstRay(Ray, T2);
			if (!Test.Hit)
				print("warning: can't find plane to place entity!");
		}

		return Test;
	}

	// --------------
	// >> PLACE MODE
	// --------------
	void ActivatePlaceMode(EEntity* Entity)
	{
		auto& EdContext = *GetContext();

		DeactivateEditorModes();
		EdContext.PlaceMode = true;
		EdContext.SelectedEntity = Entity;
		EdContext.UndoStack.Track(Entity);
	}

	void SelectEntityPlacingWithMouseMove(EEntity* Entity, const RWorld* World)
	{
		auto* GII = GlobalInputInfo::Get();

		auto Pickray = CastPickray(RCameraManager::Get()->GetCurrentCamera(), GII->MouseCoords.X, GII->MouseCoords.Y);
		auto Test = World->Raycast(Pickray, Entity);
		if (Test.Hit)
		{
			Entity->Position = ClGetPointFromDetection(Pickray, Test);
			Entity->Update();
		}
	}


	// -------------
	// >> MOVE MODE
	// -------------
	void ActivateMoveMode(EEntity* Entity)
	{
		auto& EdContext = *GetContext();

		DeactivateEditorModes();
		EdContext.MoveMode = true;
		EdContext.MoveAxis = 0;
		EdContext.SelectedEntity = Entity;
		EdContext.UndoStack.Track(Entity);
	}

	void MoveEntityWithMouse(EEntity* Entity)
	{
		auto& EdContext = *GetContext();

		RRaycastTest Test = TestRayAgainstEntitySupportPlane(EdContext.MoveAxis, Entity);
		if (!Test.Hit)
			return;

		RRay Ray = Test.Ray;
		vec3 Pos = Ray.Origin + Ray.Direction * Test.Distance;

		// places entity accordingly
		switch (EdContext.MoveAxis)
		{
			case 0: // XZ 
				Entity->Position.x = Pos.x;
				Entity->Position.z = Pos.z;
				break;
			case 1: // X
				Entity->Position.x = Pos.x;
				break;
			case 2: // Y
				Entity->Position.y = Pos.y;
				break;
			case 3: // Z
				Entity->Position.z = Pos.z;
				break;
		}

		Entity->Update();
	}


	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void ActivateMoveEntityByArrow(uint8 MoveAxis);
	void MoveEntityByArrows(EEntity* Entity);


	void ActivateMoveEntityByArrow(uint8 MoveAxis)
	{
		auto& EdContext = *GetContext();

		EdContext.MoveAxis = MoveAxis;
		EdContext.MoveEntityByArrows = true;
		auto Test = TestRayAgainstEntitySupportPlane(MoveAxis, EdContext.SelectedEntity);
		EdContext.MoveEntityByArrowsRefPoint = ClGetPointFromDetection(Test.Ray, Test);
		EdContext.UndoStack.Track(EdContext.SelectedEntity);
	}


	void MoveEntityByArrows(EEntity* Entity)
	{
		auto& EdContext = *GetContext();

		RRaycastTest Test = TestRayAgainstEntitySupportPlane(EdContext.MoveAxis, Entity);
		if (!Test.Hit)
			return;

		RRay Ray = Test.Ray;
		vec3 Pos = Ray.Origin + Ray.Direction * Test.Distance;

		// gets the offset from the mouse drag starting point, and not absolute Position
		vec3 Diff = Pos - EdContext.MoveEntityByArrowsRefPoint;

		// modifies entity Position
		switch (EdContext.MoveAxis)
		{
			case 0: // XZ 
				Entity->Position.x += Diff.x;
				Entity->Position.z += Diff.z;
				break;
			case 1: // X
				Entity->Position.x += Diff.x;
				break;
			case 2: // Y
				Entity->Position.y += Diff.y;
				break;
			case 3: // Z
				Entity->Position.z += Diff.z;
				break;
		}

		EdContext.MoveEntityByArrowsRefPoint = Pos;

		Entity->Update();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&EdContext.EntityPanel);
		UpdateEntityRotationGizmo(&EdContext.EntityPanel);
	}

	// ----------------
	// MOVE LIGHT TOOL
	// ----------------
	// @todo: This will DISAPPEAR after lights become entities!
	//       We need to provide entity rights to lights too! revolution now!

	void MoveLightWithMouse(std::string Type, int Index, RWorld* World);
	void ActivateMoveLightMode(std::string Type, int Index);
	void PlaceLight(std::string Type, int Index);
	void OpenLightsPanel(std::string Type, int Index, bool FocusTab); //fwd


	void ActivateMoveLightMode(std::string Type, int Index)
	{
		auto& EdContext = *GetContext();

		DeactivateEditorModes();
		EdContext.EntityPanel.Active = false;
		OpenLightsPanel(Type, Index, true);
		EdContext.MoveMode = true;
		EdContext.SelectedLight = Index;
		EdContext.SelectedLightType = Type;
	}

	void MoveLightWithMouse(std::string Type, int Index, RWorld* World)
	{
		vec3 Position;
		if (Type == "point" && Index > -1)
			Position = World->PointLights[Index]->Position;
		else if (Type == "spot" && Index > -1)
			Position = World->SpotLights[Index]->Position;
		else
			assert(false);

		auto* GII = GlobalInputInfo::Get();
		auto* Camera = RCameraManager::Get()->GetCurrentCamera();
		auto Ray = CastPickray(Camera, GII->MouseCoords.X, GII->MouseCoords.Y);

		// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
		// Position. In the case of Y placement, we need to compute the plane considering the camera orientation.
		RTriangle T1, T2;
		float PlaneSize = 500.0f;

		auto& EdContext = *GetContext();

		switch (EdContext.MoveAxis)
		{
			case 0: // XZ 
			case 1: // X
			case 3: // Z
				T1.A = vec3{Position.x - PlaneSize, Position.y, Position.z - PlaneSize};
				T1.B = vec3{Position.x + PlaneSize, Position.y, Position.z - PlaneSize};
				T1.C = vec3{Position.x + PlaneSize, Position.y, Position.z + PlaneSize};
				T2.A = vec3{Position.x - PlaneSize, Position.y, Position.z - PlaneSize};
				T2.B = vec3{Position.x - PlaneSize, Position.y, Position.z + PlaneSize};
				T2.C = vec3{Position.x + PlaneSize, Position.y, Position.z + PlaneSize};
				break;
			case 2: // Y
			{
				// creates vector from cam to entity in XZ
				vec3 CamToEntity = Camera->Position - Position;
				CamToEntity.y = Camera->Position.y;
				CamToEntity = normalize(CamToEntity);
				// finds vector that lie in plane considering cam to entity vector as normal to it
				vec3 UpVec = normalize(vec3{Camera->Position.x, 1.0f, Camera->Position.z});
				vec3 VecInPlane = glm::cross(UpVec, CamToEntity);

				// creates plane
				T1.A = Position + (VecInPlane * -1.0f * PlaneSize);
				T1.A.y = Camera->Position.y + -1.0f * PlaneSize;

				T1.B = Position + (VecInPlane * PlaneSize);
				T1.B.y = Camera->Position.y + -1.0f * PlaneSize;

				T1.C = Position + (VecInPlane * PlaneSize);
				T1.C.y = Camera->Position.y + PlaneSize;

				T2.A = T1.A;
				T2.B = Position + (VecInPlane * -1.0f * PlaneSize);
				T2.B.y = Camera->Position.y + PlaneSize;
				T2.C = T1.C;

				break;
			}
		}

		// ray casts against created plane
		RRaycastTest Test;

		Test = ClTestAgainstRay(Ray, T1);
		if (!Test.Hit)
		{
			Test = ClTestAgainstRay(Ray, T2);
			if (!Test.Hit)
			{
				print("warning: can't find plane to place light!");
				return;
			}
		}

		// places entity accordingly
		switch (EdContext.MoveAxis)
		{
			case 0:
				Position.x = Ray.Origin.x + Ray.Direction.x * Test.Distance;
				Position.z = Ray.Origin.z + Ray.Direction.z * Test.Distance;
				break;
			case 1: // X
				Position.x = Ray.Origin.x + Ray.Direction.x * Test.Distance;
				break;
			case 2: // Y
				Position.y = Ray.Origin.y + Ray.Direction.y * Test.Distance;
				break;
			case 3: // Z
				Position.z = Ray.Origin.z + Ray.Direction.z * Test.Distance;
				break;
		}

		if (Type == "point" && Index > -1)
			World->PointLights[Index]->Position = Position;
		else if (Type == "spot" && Index > -1)
			World->SpotLights[Index]->Position = Position;
		else
			assert(false);
	}

	void PlaceLight()
	{
		auto& EdContext = *GetContext();

		EdContext.MoveMode = false;
		EdContext.SelectedLight = -1;
	}


	// ---------------------
	// > ROTATE ENTITY TOOL
	// ---------------------
	void ActivateRotateEntityWithMouse(uint8 MoveAxis);
	float MouseOffsetToAngularOffset(float MouseOffset);
	void RotateEntityWithMouse(EEntity* Entity);

	void ActivateRotateEntityWithMouse(uint8 MoveAxis)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& EdContext = *GetContext();

		EdContext.MoveAxis = MoveAxis;
		EdContext.RotateEntityWithMouse = true;
		EdContext.RotateEntityWithMouseMouseCoordsRef = vec2(
			GII->MouseCoords.X,
			GII->MouseCoords.Y
		);
		EdContext.UndoStack.Track(EdContext.SelectedEntity);
	}

	float MouseOffsetToAngularOffset(float MouseOffset)
	{
		// 360 degrees per 500 pixels of offset
		return MouseOffset * 360.f / 500.f;
	}

	void RotateEntityWithMouse(EEntity* Entity)
	{
		auto* GII = GlobalInputInfo::Get();
		auto MouseCoords = vec2(GII->MouseCoords.X, GII->MouseCoords.Y);
		auto& EdContext = *GetContext();

		switch (EdContext.MoveAxis)
		{
			case 1: // X
			{
				float Diff = MouseCoords.y - EdContext.RotateEntityWithMouseMouseCoordsRef.y;
				float AngularDiff = MouseOffsetToAngularOffset(Diff);
				Entity->Rotation.x += AngularDiff;
				break;
			}
			case 2: // Y
			{
				float Diff = MouseCoords.x - EdContext.RotateEntityWithMouseMouseCoordsRef.x;
				float AngularDiff = MouseOffsetToAngularOffset(Diff);
				Entity->Rotation.y += AngularDiff;
				break;
			}
			case 3: // Z
			{
				float Diff = MouseCoords.y - EdContext.RotateEntityWithMouseMouseCoordsRef.y;
				float AngularDiff = MouseOffsetToAngularOffset(Diff);
				Entity->Rotation.z += AngularDiff;
				break;
			}
		}

		EdContext.RotateEntityWithMouseMouseCoordsRef = MouseCoords;
		Entity->Update();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&EdContext.EntityPanel);
		UpdateEntityRotationGizmo(&EdContext.EntityPanel);
	}


	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void ScaleEntityWithMouse(EEntity* Entity)
	{
		// NOT IMPLEMENTED
	}


	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void ActivateSelectEntityAuxTool(EEntity** EntitySlot, NEdToolCallback Callback, REditorToolCallbackArgs Args)
	{
		auto& EdContext = *GetContext();

		EdContext.SelectEntityAuxMode = true;
		EdContext.SelectEntityAuxModeEntitySlot = EntitySlot;
		EdContext.SelectEntityAuxModeCallback = Callback;
		EdContext.SelectEntityAuxModeCallbackArgs = Args;
	}


	// -------------
	// MISCELANEOUS
	// -------------
	// void CheckForAssetChanges();
	void RenderAabbBoundaries(EEntity* Entity);

	// void CheckForAssetChanges()
	// {
	//    auto it = Geometry_Catalogue.begin();
	//    while (it != Geometry_Catalogue.end())
	//    {
	//       auto model_name = it->first;
	//       std::string path = Paths::Models + model_name + ".obj";

	//       //@todo: platform dependency
	//       WIN32_FIND_DATA find_data;
	//       HANDLE find_handle = FindFirstFileA(path.c_str(), &find_data);
	//       if(find_handle != INVALID_HANDLE_VALUE)
	//       {
	//          auto mesh = it->second;
	//          if(CompareFileTime(&mesh->last_written, &find_data.ftLastWriteTime) != 0)
	//          {
	//             log(LOG_INFO, "asset updated: " + model_name);
	//             mesh->last_written = find_data.ftLastWriteTime;
	//             auto dummy_mesh = load_wavefront_obj_as_mesh(Paths::Models, model_name);
	//             mesh->vertices = dummy_mesh->vertices;
	//             mesh->indices = dummy_mesh->indices;
	//             mesh->gl_data = dummy_mesh->gl_data;
	//             free(dummy_mesh);
	//          }
	//       }

	//       FindClose(find_handle);
	//       it++;
	//    }
	// }

	void RenderAabbBoundaries(EEntity* Entity)
	{
		// auto bounds = Entity->collision_geometry.aabb;
		// ImDraw::add(
		//    IMHASH,
		//    std::vector<Vertex>{
		//       Vertex{vec3(bounds.x0,Entity->Position.y, bounds.z0)},
		//       Vertex{vec3(bounds.x0,Entity->Position.y + bounds.height, bounds.z0)},

		//       Vertex{vec3(bounds.x0,Entity->Position.y, bounds.z1)},
		//       Vertex{vec3(bounds.x0,Entity->Position.y + bounds.height, bounds.z1)},

		//       Vertex{vec3(bounds.x1,Entity->Position.y, bounds.z1)},
		//       Vertex{vec3(bounds.x1,Entity->Position.y + bounds.height, bounds.z1)},

		//       Vertex{vec3(bounds.x1,Entity->Position.y, bounds.z0)},
		//       Vertex{vec3(bounds.x1,Entity->Position.y + bounds.height, bounds.z0)},
		//    },
		//    GL_POINTS
		// );
	}
}
