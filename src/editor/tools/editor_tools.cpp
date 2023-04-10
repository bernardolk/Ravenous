#pragma once

#include "editor_tools.h"
#include "editor/editor.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/collision/cl_controller.h"
#include "engine/io/input.h"
#include "engine/world/scene_manager.h"
#include "engine/world/world_chunk.h"

namespace Editor
{
	void DeactivateEditorModes()
	{
		auto& ed_context = *GetContext();

		ed_context.move_mode = false;
		ed_context.snap_mode = false;
		ed_context.measure_mode = false;
		ed_context.first_point_found = false;
		ed_context.second_point_found = false;
		ed_context.stretch_mode = false;
		ed_context.locate_coords_mode = false;
		ed_context.place_mode = false;
		ed_context.select_entity_aux_mode = false;
	}

	bool CheckModesAreActive()
	{
		auto& ed_context = *GetContext();

		return
		ed_context.move_mode ||
		ed_context.snap_mode ||
		ed_context.measure_mode ||
		ed_context.stretch_mode ||
		ed_context.locate_coords_mode;
	}

	void EditorEraseEntity(E_Entity* entity)
	{
		//TODO: To be implemented
		auto& ed_context = *GetContext();

		ed_context.undo_stack.deletion_log.Add(entity);
	}

	void EditorEraseLight(int index, string type, T_World* world)
	{
		auto& ed_context = *GetContext();

		if (type == "point")
		{
			world->point_lights.erase(world->point_lights.begin() + index);
		}
		else if (type == "spot")
		{
			world->spot_lights.erase(world->spot_lights.begin() + index);
		}

		if (ed_context.lights_panel.selected_light == index)
			ed_context.lights_panel.selected_light = -1;
	}

	void UnhideEntities(T_World* world)
	{
		auto chunk_iterator = world->GetChunkIterator();
		while (auto* chunk = chunk_iterator())
		{
			auto entity_iterator = chunk->GetIterator();
			while (auto* entity = entity_iterator())
			{
				if (entity->flags & EntityFlags_HiddenEntity)
					entity->flags &= ~EntityFlags_HiddenEntity;
			}
		}
	}

	// ----------
	// SNAP TOOL
	// ----------
	void ActivateSnapMode(E_Entity* entity);
	void SnapEntityToReference(E_Entity* entity);
	void CheckSelectionToSnap();
	void SnapCommit();

	void ActivateSnapMode(E_Entity* entity)
	{
		// deactivate_editor_modes();
		// ed_context.snap_mode = true;
		// ed_context.snap_tracked_state = get_entity_state(entity);
		// ed_context.undo_stack.track(entity);
	}

	void SnapCommit()
	{
		// auto entity = ed_context.entity_panel.entity;
		// ed_context.snap_tracked_state = get_entity_state(entity);
		// ed_context.undo_stack.track(entity);
	}

	void SnapEntityToReference(E_Entity* entity)
	{
		// auto reference = ed_context.snap_reference;
		// float diff = 0;
		// auto diff_vec = vec3(0.0f);
		// auto [x0, x1, z0, z1]         = reference->get_rect_bounds();
		// auto [e_x0, e_x1, e_z0, e_z1] = entity->get_rect_bounds();

		// switch(ed_context.snap_axis)
		// {
		//    case 0:  // x
		//       if     (ed_context.snap_cycle == 0 && !ed_context.snap_inside) diff_vec.x = x1 - e_x0;
		//       else if(ed_context.snap_cycle == 0 &&  ed_context.snap_inside) diff_vec.x = x1 - e_x0 - (e_x1 - e_x0);
		//       else if(ed_context.snap_cycle == 2 && !ed_context.snap_inside) diff_vec.x = x0 - e_x1;
		//       else if(ed_context.snap_cycle == 2 &&  ed_context.snap_inside) diff_vec.x = x0 - e_x1 + (e_x1 - e_x0);
		//       else if(ed_context.snap_cycle == 1 ) diff_vec.x = x1 - e_x1 - (x1 - x0) / 2.0 + (e_x1 - e_x0) / 2.0;
		//       break;
		//    case 1:  // y
		//    {
		//       float bottom = reference->position.y;
		//       float height = reference->get_height();
		//       float top = bottom + height;
		//       float current_bottom = entity->position.y;
		//       float current_top = current_bottom + entity->get_height();

		//       if     (ed_context.snap_cycle == 0 && !ed_context.snap_inside) diff_vec.y = top - current_bottom;
		//       else if(ed_context.snap_cycle == 0 &&  ed_context.snap_inside) diff_vec.y = top - current_top;
		//       else if(ed_context.snap_cycle == 2 && !ed_context.snap_inside) diff_vec.y = bottom - current_top;
		//       else if(ed_context.snap_cycle == 2 &&  ed_context.snap_inside) diff_vec.y = bottom - current_bottom;
		//       else if(ed_context.snap_cycle == 1 && !ed_context.snap_inside) diff_vec.y = top - height / 2.0 - current_top;
		//       break;
		//    }
		//    case 2: // z
		//       if     (ed_context.snap_cycle == 0 && !ed_context.snap_inside) diff_vec.z = z1 - e_z0;
		//       else if(ed_context.snap_cycle == 0 &&  ed_context.snap_inside) diff_vec.z = z1 - e_z0 - (e_z1 - e_z0);
		//       else if(ed_context.snap_cycle == 2 && !ed_context.snap_inside) diff_vec.z = z0 - e_z1;
		//       else if(ed_context.snap_cycle == 2 &&  ed_context.snap_inside) diff_vec.z = z0 - e_z1 + (e_z1 - e_z0);
		//       else if(ed_context.snap_cycle == 1 ) diff_vec.z = z1 - e_z1 - (z1 - z0) / 2.0 + (e_z1 - e_z0) / 2.0;
		//       break;
		// }

		// entity->position += diff_vec;
	}


	void CheckSelectionToSnap()
	{
		// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.mouse_coords.x, G_INPUT_INFO.mouse_coords.y);
		// auto test = test_ray_against_scene(pickray);
		// if(test.hit)
		// {
		//    ed_context.snap_reference = test.entity;
		//    snap_entity_to_reference(ed_context.entity_panel.entity);
		// }
	}

	// -------------
	// STRETCH TOOL
	// -------------
	void ActivateStretchMode(E_Entity* entity);
	void StretchCommit();
	auto GetScaleAndPositionChange(E_Entity* entity, float old_pos, float new_pos, float n);
	void StretchEntityToReference(E_Entity* entity);
	void check_selection_to_stretch(EntityPanelContext* panel);

	void ActivateStretchMode(E_Entity* entity)
	{
		// deactivate_editor_modes();
		// ed_context.stretch_mode = true;
		// ed_context.undo_stack.track(entity);
	}

	void StretchCommit()
	{
		// auto entity = ed_context.entity_panel.entity;
		// ed_context.undo_stack.track(entity);
	}

	auto get_scale_and_position_change(float e_scale, float e_aligned, float e_opposite, float t, float n)
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
		} transform;

		float dif = e_aligned - t;

		bool shrink = (dif > 0 && n > 0) || (dif < 0 && n < 0);

		// if we are going to invert the scale of the entity by shrinking it towards itself,
		// then we stretch using the other side of the entity to achieve expected behaviour.
		if (shrink && abs(dif) >= e_scale)
		{
			dif = e_opposite - t;
			n *= -1;
			shrink = false;
		}

		if (shrink)
			transform.scale_f = -1.0 * abs(dif);
		else
			transform.scale_f = abs(dif);

		// if normal points to negative dir, move position
		if (n < 0)
			transform.pos_f -= dif;

		return transform;
	}

	void stretch_entity_to_reference(E_Entity* entity, Triangle t)
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
		// int triangles = entity->mesh->indices.size() / 3;
		// for(int i = 0; i < triangles; i++)
		// {
		//    Triangle _t = get_triangle_for_indexed_mesh(entity->mesh, entity->matModel, i);
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
		//    auto [s, p] = get_scale_and_position_change(entity->scale.x, e_aligned.a.x, e_opposite.a.x, t.a.x, normal.x);
		//    entity->scale.x += s;
		//    entity->position.x += p;
		//    axis_count++;
		// }
		// if(!is_zero(normal.y))
		// {
		//    auto [s, p] = get_scale_and_position_change(entity->scale.y, e_aligned.a.y, e_opposite.a.y, t.a.y, normal.y);
		//    entity->scale.y += s;
		//    entity->position.y += p;
		//    axis_count++;
		// }
		// if(!is_zero(normal.z))
		// {
		//    auto [s, p] = get_scale_and_position_change(entity->scale.z, e_aligned.a.z, e_opposite.a.z, t.a.z, normal.z);
		//    entity->scale.z += s;
		//    entity->position.z += p;
		//    axis_count++;
		// }

		// assert(axis_count == 1);
		// return;
	}



	void CheckSelectionToStretch()
	{
		// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.mouse_coords.x, G_INPUT_INFO.mouse_coords.y);
		// auto test = test_ray_against_scene(pickray);
		// if(test.hit)
		// {
		//    stretch_entity_to_reference(ed_context.entity_panel.entity, test.t);
		//    stretch_commit();
		// }
	}


	// -------------
	// MEASURE TOOL
	// -------------
	void ActivateMeasureMode(u8 axis);
	void CheckSelectionToMeasure(const T_World* world);

	void ActivateMeasureMode(u8 axis)
	{
		auto& ed_context = *GetContext();

		DeactivateEditorModes();
		ed_context.measure_mode = true;
		ed_context.measure_axis = axis;
	}

	void CheckSelectionToMeasure(const T_World* world)
	{
		auto* GII = GlobalInputInfo::Get();
		auto* GSI = GlobalSceneInfo::Get();
		auto& ed_context = *GetContext();

		auto pickray = CastPickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray);
		if (test.hit)
		{
			if (!ed_context.first_point_found || ed_context.second_point_found)
			{
				if (ed_context.second_point_found)
					ed_context.second_point_found = false;
				ed_context.first_point_found = true;
				ed_context.measure_from = CL_GetPointFromDetection(pickray, test);
			}
			else if (!ed_context.second_point_found)
			{
				ed_context.second_point_found = true;
				vec3 point = CL_GetPointFromDetection(pickray, test);
				if (ed_context.measure_axis == 0)
					ed_context.measure_to = point.x;
				else if (ed_context.measure_axis == 1)
					ed_context.measure_to = point.y;
				else if (ed_context.measure_axis == 2)
					ed_context.measure_to = point.z;
			}
		}
	}

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void ActivateLocateCoordsMode();
	void CheckSelectionToLocateCoords(const T_World* world);

	void ActivateLocateCoordsMode()
	{
		auto& ed_context = *GetContext();

		DeactivateEditorModes();
		ed_context.locate_coords_mode = true;
		ed_context.locate_coords_found_point = false;
	}

	void CheckSelectionToLocateCoords(const T_World* world)
	{
		auto* GII = GlobalInputInfo::Get();
		auto* GSI = GlobalSceneInfo::Get();
		auto& ed_context = *GetContext();

		auto pickray = CastPickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray);
		if (test.hit)
		{
			ed_context.locate_coords_found_point = true;
			ed_context.locate_coords_position = CL_GetPointFromDetection(pickray, test);
		}
	}

	// -------------
	// > MOVE TOOLS 
	// -------------
	void PlaceEntity(T_World* world)
	{
		/* Common function for move/rotate/scale entity tools.
		   Updates entity, tracks it state and updates world.
		   To be called at the end of entity modification operation. */
		auto* GSI = GlobalSceneInfo::Get();
		auto& ed_context = *GetContext();

		ed_context.move_mode = false;
		ed_context.move_entity_by_arrows = false;
		ed_context.rotate_entity_with_mouse = false;
		ed_context.place_mode = false;
		ed_context.move_entity_by_arrows_ref_point = vec3(0);

		ed_context.selected_entity->Update();
		world->UpdateEntityWorldCells(ed_context.selected_entity);
		CL_RecomputeCollisionBufferEntities(GSI->player);
		ed_context.undo_stack.Track(ed_context.selected_entity);
	}

	RaycastTest TestRayAgainstEntitySupportPlane(u16 move_axis, E_Entity* entity)
	{
		// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
		// position. In the case of Y placement, we need to compute the plane considering the camera orientation.
		Triangle t1, t2;
		float plane_size = 500.0f;
		auto* GSI = GlobalSceneInfo::Get();
		auto& ed_context = *GetContext();

		switch (ed_context.move_axis)
		{
			case 0: // XZ 
			case 1: // X
			case 3: // Z
				t1.a = vec3{entity->position.x - plane_size, entity->position.y, entity->position.z - plane_size};
				t1.b = vec3{entity->position.x + plane_size, entity->position.y, entity->position.z - plane_size};
				t1.c = vec3{entity->position.x + plane_size, entity->position.y, entity->position.z + plane_size};
				t2.a = vec3{entity->position.x - plane_size, entity->position.y, entity->position.z - plane_size};
				t2.b = vec3{entity->position.x - plane_size, entity->position.y, entity->position.z + plane_size};
				t2.c = vec3{entity->position.x + plane_size, entity->position.y, entity->position.z + plane_size};
				break;
			case 2: // Y
			{
				// creates vector from cam to entity in XZ
				auto camera = GSI->camera;
				vec3 cam_to_entity = camera->position - entity->position;
				cam_to_entity.y = camera->position.y;
				cam_to_entity = normalize(cam_to_entity);
				// finds vector that lie in plane considering cam to entity vector as normal to it
				vec3 up_vec = normalize(vec3{camera->position.x, 1.0f, camera->position.z});
				vec3 vec_in_plane = glm::cross(up_vec, cam_to_entity);

				// creates plane
				t1.a = entity->position + (vec_in_plane * -1.0f * plane_size);
				t1.a.y = camera->position.y + -1.0f * plane_size;

				t1.b = entity->position + (vec_in_plane * plane_size);
				t1.b.y = camera->position.y + -1.0f * plane_size;

				t1.c = entity->position + (vec_in_plane * plane_size);
				t1.c.y = camera->position.y + plane_size;

				t2.a = t1.a;
				t2.b = entity->position + (vec_in_plane * -1.0f * plane_size);
				t2.b.y = camera->position.y + plane_size;
				t2.c = t1.c;

				break;
			}
		}

		// ray casts against created plane
		auto* GII = GlobalInputInfo::Get();

		auto ray = CastPickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);
		RaycastTest test;

		test = CL_TestAgainstRay(ray, t1);
		if (!test.hit)
		{
			test = CL_TestAgainstRay(ray, t2);
			if (!test.hit)
				std::cout << "warning: can't find plane to place entity!\n";
		}

		return test;
	}

	// --------------
	// >> PLACE MODE
	// --------------
	void ActivatePlaceMode(E_Entity* entity)
	{
		auto& ed_context = *GetContext();

		DeactivateEditorModes();
		ed_context.place_mode = true;
		ed_context.selected_entity = entity;
		ed_context.undo_stack.Track(entity);
	}

	void SelectEntityPlacingWithMouseMove(E_Entity* entity, const T_World* world)
	{
		auto* GII = GlobalInputInfo::Get();
		auto* GSI = GlobalSceneInfo::Get();

		auto pickray = CastPickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, entity);
		if (test.hit)
		{
			entity->position = CL_GetPointFromDetection(pickray, test);
			entity->Update();
		}
	}


	// -------------
	// >> MOVE MODE
	// -------------
	void ActivateMoveMode(E_Entity* entity)
	{
		auto& ed_context = *GetContext();

		DeactivateEditorModes();
		ed_context.move_mode = true;
		ed_context.move_axis = 0;
		ed_context.selected_entity = entity;
		ed_context.undo_stack.Track(entity);
	}

	void MoveEntityWithMouse(E_Entity* entity)
	{
		auto& ed_context = *GetContext();

		RaycastTest test = TestRayAgainstEntitySupportPlane(ed_context.move_axis, entity);
		if (!test.hit)
			return;

		Ray ray = test.ray;
		vec3 pos = ray.origin + ray.direction * test.distance;

		// places entity accordingly
		switch (ed_context.move_axis)
		{
			case 0: // XZ 
				entity->position.x = pos.x;
				entity->position.z = pos.z;
				break;
			case 1: // X
				entity->position.x = pos.x;
				break;
			case 2: // Y
				entity->position.y = pos.y;
				break;
			case 3: // Z
				entity->position.z = pos.z;
				break;
		}

		entity->Update();
	}


	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void ActivateMoveEntityByArrow(u8 move_axis);
	void MoveEntityByArrows(Entity* entity);


	void ActivateMoveEntityByArrow(u8 move_axis)
	{
		auto& ed_context = *GetContext();

		ed_context.move_axis = move_axis;
		ed_context.move_entity_by_arrows = true;
		auto test = TestRayAgainstEntitySupportPlane(move_axis, ed_context.selected_entity);
		ed_context.move_entity_by_arrows_ref_point = CL_GetPointFromDetection(test.ray, test);
		ed_context.undo_stack.Track(ed_context.selected_entity);
	}


	void MoveEntityByArrows(E_Entity* entity)
	{
		auto& ed_context = *GetContext();

		RaycastTest test = TestRayAgainstEntitySupportPlane(ed_context.move_axis, entity);
		if (!test.hit)
			return;

		Ray ray = test.ray;
		vec3 pos = ray.origin + ray.direction * test.distance;

		// gets the offset from the mouse drag starting point, and not absolute position
		vec3 diff = pos - ed_context.move_entity_by_arrows_ref_point;

		// modifies entity position
		switch (ed_context.move_axis)
		{
			case 0: // XZ 
				entity->position.x += diff.x;
				entity->position.z += diff.z;
				break;
			case 1: // X
				entity->position.x += diff.x;
				break;
			case 2: // Y
				entity->position.y += diff.y;
				break;
			case 3: // Z
				entity->position.z += diff.z;
				break;
		}

		ed_context.move_entity_by_arrows_ref_point = pos;

		entity->Update();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&ed_context.entity_panel);
		UpdateEntityRotationGizmo(&ed_context.entity_panel);
	}



	// ----------------
	// MOVE LIGHT TOOL
	// ----------------
	// @todo: This will DISAPPEAR after lights become entities!
	//       We need to provide entity rights to lights too! revolution now!

	void MoveLightWithMouse(std::string type, int index, T_World* world);
	void ActivateMoveLightMode(std::string type, int index);
	void PlaceLight(std::string type, int index);
	void OpenLightsPanel(std::string type, int index, bool focus_tab); //fwd


	void ActivateMoveLightMode(std::string type, int index)
	{
		auto& ed_context = *GetContext();

		DeactivateEditorModes();
		ed_context.entity_panel.active = false;
		OpenLightsPanel(type, index, true);
		ed_context.move_mode = true;
		ed_context.selected_light = index;
		ed_context.selected_light_type = type;
	}

	void MoveLightWithMouse(std::string type, int index, T_World* world)
	{
		vec3 position;
		if (type == "point" && index > -1)
			position = world->point_lights[index]->position;
		else if (type == "spot" && index > -1)
			position = world->spot_lights[index]->position;
		else
			assert(false);

		auto* GII = GlobalInputInfo::Get();
		auto* GSI = GlobalSceneInfo::Get();

		auto ray = CastPickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);

		// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
		// position. In the case of Y placement, we need to compute the plane considering the camera orientation.
		Triangle t1, t2;
		float plane_size = 500.0f;

		auto& ed_context = *GetContext();

		switch (ed_context.move_axis)
		{
			case 0: // XZ 
			case 1: // X
			case 3: // Z
				t1.a = vec3{position.x - plane_size, position.y, position.z - plane_size};
				t1.b = vec3{position.x + plane_size, position.y, position.z - plane_size};
				t1.c = vec3{position.x + plane_size, position.y, position.z + plane_size};
				t2.a = vec3{position.x - plane_size, position.y, position.z - plane_size};
				t2.b = vec3{position.x - plane_size, position.y, position.z + plane_size};
				t2.c = vec3{position.x + plane_size, position.y, position.z + plane_size};
				break;
			case 2: // Y
			{
				// creates vector from cam to entity in XZ
				auto camera = GSI->camera;
				vec3 cam_to_entity = camera->position - position;
				cam_to_entity.y = camera->position.y;
				cam_to_entity = normalize(cam_to_entity);
				// finds vector that lie in plane considering cam to entity vector as normal to it
				vec3 up_vec = normalize(vec3{camera->position.x, 1.0f, camera->position.z});
				vec3 vec_in_plane = glm::cross(up_vec, cam_to_entity);

				// creates plane
				t1.a = position + (vec_in_plane * -1.0f * plane_size);
				t1.a.y = camera->position.y + -1.0f * plane_size;

				t1.b = position + (vec_in_plane * plane_size);
				t1.b.y = camera->position.y + -1.0f * plane_size;

				t1.c = position + (vec_in_plane * plane_size);
				t1.c.y = camera->position.y + plane_size;

				t2.a = t1.a;
				t2.b = position + (vec_in_plane * -1.0f * plane_size);
				t2.b.y = camera->position.y + plane_size;
				t2.c = t1.c;

				break;
			}
		}

		// ray casts against created plane
		RaycastTest test;

		test = CL_TestAgainstRay(ray, t1);
		if (!test.hit)
		{
			test = CL_TestAgainstRay(ray, t2);
			if (!test.hit)
			{
				std::cout << "warning: can't find plane to place light!\n";
				return;
			}
		}

		// places entity accordingly
		switch (ed_context.move_axis)
		{
			case 0:
				position.x = ray.origin.x + ray.direction.x * test.distance;
				position.z = ray.origin.z + ray.direction.z * test.distance;
				break;
			case 1: // X
				position.x = ray.origin.x + ray.direction.x * test.distance;
				break;
			case 2: // Y
				position.y = ray.origin.y + ray.direction.y * test.distance;
				break;
			case 3: // Z
				position.z = ray.origin.z + ray.direction.z * test.distance;
				break;
		}

		if (type == "point" && index > -1)
			world->point_lights[index]->position = position;
		else if (type == "spot" && index > -1)
			world->spot_lights[index]->position = position;
		else
			assert(false);
	}

	void PlaceLight()
	{
		auto& ed_context = *GetContext();

		ed_context.move_mode = false;
		ed_context.selected_light = -1;
	}


	// ---------------------
	// > ROTATE ENTITY TOOL
	// ---------------------
	void ActivateRotateEntityWithMouse(u8 move_axis);
	float MouseOffsetToAngularOffset(float mouse_offset);
	void RotateEntityWithMouse(Entity* entity);

	void ActivateRotateEntityWithMouse(u8 move_axis)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& ed_context = *GetContext();

		ed_context.move_axis = move_axis;
		ed_context.rotate_entity_with_mouse = true;
		ed_context.rotate_entity_with_mouse_mouse_coords_ref = vec2(
			GII->mouse_coords.x,
			GII->mouse_coords.y
		);
		ed_context.undo_stack.Track(ed_context.selected_entity);
	}

	float MouseOffsetToAngularOffset(float mouse_offset)
	{
		// 360 degrees per 500 pixels of offset
		return mouse_offset * 360.f / 500.f;
	}

	void RotateEntityWithMouse(E_Entity* entity)
	{
		auto* GII = GlobalInputInfo::Get();
		auto mouse_coords = vec2(GII->mouse_coords.x, GII->mouse_coords.y);
		auto& ed_context = *GetContext();

		switch (ed_context.move_axis)
		{
			case 1: // X
			{
				float diff = mouse_coords.y - ed_context.rotate_entity_with_mouse_mouse_coords_ref.y;
				float angular_diff = MouseOffsetToAngularOffset(diff);
				entity->rotation.x += angular_diff;
				break;
			}
			case 2: // Y
			{
				float diff = mouse_coords.x - ed_context.rotate_entity_with_mouse_mouse_coords_ref.x;
				float angular_diff = MouseOffsetToAngularOffset(diff);
				entity->rotation.y += angular_diff;
				break;
			}
			case 3: // Z
			{
				float diff = mouse_coords.y - ed_context.rotate_entity_with_mouse_mouse_coords_ref.y;
				float angular_diff = MouseOffsetToAngularOffset(diff);
				entity->rotation.z += angular_diff;
				break;
			}
		}

		ed_context.rotate_entity_with_mouse_mouse_coords_ref = mouse_coords;
		entity->Update();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&ed_context.entity_panel);
		UpdateEntityRotationGizmo(&ed_context.entity_panel);
	}


	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void ScaleEntityWithMouse(E_Entity* entity)
	{
		// NOT IMPLEMENTED
	}


	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void ActivateSelectEntityAuxTool(E_Entity** entity_slot, EdToolCallback callback, EdToolCallbackArgs args)
	{
		auto& ed_context = *GetContext();

		ed_context.select_entity_aux_mode = true;
		ed_context.select_entity_aux_mode_entity_slot = entity_slot;
		ed_context.select_entity_aux_mode_callback = callback;
		ed_context.select_entity_aux_mode_callback_args = args;
	}


	// -------------
	// MISCELANEOUS
	// -------------
	// void CheckForAssetChanges();
	void RenderAabbBoundaries(Entity* entity);

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

	void RenderAabbBoundaries(Entity* entity)
	{
		// auto bounds = entity->collision_geometry.aabb;
		// ImDraw::add(
		//    IMHASH,
		//    std::vector<Vertex>{
		//       Vertex{vec3(bounds.x0,entity->position.y, bounds.z0)},
		//       Vertex{vec3(bounds.x0,entity->position.y + bounds.height, bounds.z0)},

		//       Vertex{vec3(bounds.x0,entity->position.y, bounds.z1)},
		//       Vertex{vec3(bounds.x0,entity->position.y + bounds.height, bounds.z1)},

		//       Vertex{vec3(bounds.x1,entity->position.y, bounds.z1)},
		//       Vertex{vec3(bounds.x1,entity->position.y + bounds.height, bounds.z1)},

		//       Vertex{vec3(bounds.x1,entity->position.y, bounds.z0)},
		//       Vertex{vec3(bounds.x1,entity->position.y + bounds.height, bounds.z0)},
		//    },
		//    GL_POINTS
		// );
	}
}
