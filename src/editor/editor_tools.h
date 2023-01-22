#pragma once

inline void deactivate_editor_modes()
{
	EdContext.move_mode = false;
	EdContext.snap_mode = false;
	EdContext.measure_mode = false;
	EdContext.first_point_found = false;
	EdContext.second_point_found = false;
	EdContext.stretch_mode = false;
	EdContext.locate_coords_mode = false;
	EdContext.place_mode = false;
	EdContext.select_entity_aux_mode = false;
}

inline bool check_modes_are_active()
{
	return
	EdContext.move_mode ||
	EdContext.snap_mode ||
	EdContext.measure_mode ||
	EdContext.stretch_mode ||
	EdContext.locate_coords_mode;
}

inline void editor_erase_entity(Entity* entity)
{
	auto* EM = EntityManager::Get();
	EM->MarkForDeletion(entity);
	EdContext.undo_stack.deletion_log.Add(entity);
}

inline void editor_erase_light(int index, std::string type, World* world)
{
	if(type == "point")
	{
		world->point_lights.erase(world->point_lights.begin() + index);
	}
	else if(type == "spot")
	{
		world->spot_lights.erase(world->spot_lights.begin() + index);
	}

	if(EdContext.lights_panel.selected_light == index)
		EdContext.lights_panel.selected_light = -1;
}

inline void unhide_entities(const World* world)
{
	for(auto& entity : world->entities)
	{
		if(entity->flags & EntityFlags_HiddenEntity)
			entity->flags &= ~EntityFlags_HiddenEntity;
	}
}

// ----------
// SNAP TOOL
// ----------
void activate_snap_mode(Entity* entity);
void snap_entity_to_reference(Entity* entity);
void check_selection_to_snap();
void snap_commit();

inline void activate_snap_mode(Entity* entity)
{
	// deactivate_editor_modes();
	// EdContext.snap_mode = true;
	// EdContext.snap_tracked_state = get_entity_state(entity);
	// EdContext.undo_stack.track(entity);
}

inline void snap_commit()
{
	// auto entity = EdContext.entity_panel.entity;
	// EdContext.snap_tracked_state = get_entity_state(entity);
	// EdContext.undo_stack.track(entity);
}

inline void snap_entity_to_reference(Entity* entity)
{
	// auto reference = EdContext.snap_reference;
	// float diff = 0;
	// auto diff_vec = vec3(0.0f);
	// auto [x0, x1, z0, z1]         = reference->get_rect_bounds();
	// auto [e_x0, e_x1, e_z0, e_z1] = entity->get_rect_bounds();

	// switch(EdContext.snap_axis)
	// {
	//    case 0:  // x
	//       if     (EdContext.snap_cycle == 0 && !EdContext.snap_inside) diff_vec.x = x1 - e_x0;
	//       else if(EdContext.snap_cycle == 0 &&  EdContext.snap_inside) diff_vec.x = x1 - e_x0 - (e_x1 - e_x0);
	//       else if(EdContext.snap_cycle == 2 && !EdContext.snap_inside) diff_vec.x = x0 - e_x1;
	//       else if(EdContext.snap_cycle == 2 &&  EdContext.snap_inside) diff_vec.x = x0 - e_x1 + (e_x1 - e_x0);
	//       else if(EdContext.snap_cycle == 1 ) diff_vec.x = x1 - e_x1 - (x1 - x0) / 2.0 + (e_x1 - e_x0) / 2.0;
	//       break;
	//    case 1:  // y
	//    {
	//       float bottom = reference->position.y;
	//       float height = reference->get_height();
	//       float top = bottom + height;
	//       float current_bottom = entity->position.y;
	//       float current_top = current_bottom + entity->get_height();

	//       if     (EdContext.snap_cycle == 0 && !EdContext.snap_inside) diff_vec.y = top - current_bottom;
	//       else if(EdContext.snap_cycle == 0 &&  EdContext.snap_inside) diff_vec.y = top - current_top;
	//       else if(EdContext.snap_cycle == 2 && !EdContext.snap_inside) diff_vec.y = bottom - current_top;
	//       else if(EdContext.snap_cycle == 2 &&  EdContext.snap_inside) diff_vec.y = bottom - current_bottom;
	//       else if(EdContext.snap_cycle == 1 && !EdContext.snap_inside) diff_vec.y = top - height / 2.0 - current_top;
	//       break;
	//    }
	//    case 2: // z
	//       if     (EdContext.snap_cycle == 0 && !EdContext.snap_inside) diff_vec.z = z1 - e_z0;
	//       else if(EdContext.snap_cycle == 0 &&  EdContext.snap_inside) diff_vec.z = z1 - e_z0 - (e_z1 - e_z0);
	//       else if(EdContext.snap_cycle == 2 && !EdContext.snap_inside) diff_vec.z = z0 - e_z1;
	//       else if(EdContext.snap_cycle == 2 &&  EdContext.snap_inside) diff_vec.z = z0 - e_z1 + (e_z1 - e_z0);
	//       else if(EdContext.snap_cycle == 1 ) diff_vec.z = z1 - e_z1 - (z1 - z0) / 2.0 + (e_z1 - e_z0) / 2.0;
	//       break;
	// }

	// entity->position += diff_vec;
}


inline void check_selection_to_snap()
{
	// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.mouse_coords.x, G_INPUT_INFO.mouse_coords.y);
	// auto test = test_ray_against_scene(pickray);
	// if(test.hit)
	// {
	//    EdContext.snap_reference = test.entity;
	//    snap_entity_to_reference(EdContext.entity_panel.entity);
	// }
}

// -------------
// STRETCH TOOL
// -------------
void activate_stretch_mode(Entity* entity);
void stretch_commit();
auto get_scale_and_position_change(Entity* entity, float old_pos, float new_pos, float n);
void stretch_entity_to_reference(Entity* entity);
void check_selection_to_stretch(EntityPanelContext* panel);

inline void activate_stretch_mode(Entity* entity)
{
	// deactivate_editor_modes();
	// EdContext.stretch_mode = true;
	// EdContext.undo_stack.track(entity);
}

inline void stretch_commit()
{
	// auto entity = EdContext.entity_panel.entity;
	// EdContext.undo_stack.track(entity);
}

inline auto get_scale_and_position_change(float e_scale, float e_aligned, float e_opposite, float t, float n)
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
	if(shrink && abs(dif) >= e_scale)
	{
		dif = e_opposite - t;
		n *= -1;
		shrink = false;
	}

	if(shrink)
		transform.scale_f = -1.0 * abs(dif);
	else
		transform.scale_f = abs(dif);

	// if normal points to negative dir, move position
	if(n < 0)
		transform.pos_f -= dif;

	return transform;
}

inline void stretch_entity_to_reference(Entity* entity, Triangle t)
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



inline void check_selection_to_stretch()
{
	// auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.mouse_coords.x, G_INPUT_INFO.mouse_coords.y);
	// auto test = test_ray_against_scene(pickray);
	// if(test.hit)
	// {
	//    stretch_entity_to_reference(EdContext.entity_panel.entity, test.t);
	//    stretch_commit();
	// }
}


// -------------
// MEASURE TOOL
// -------------
void activate_measure_mode(u8 axis);
void check_selection_to_measure(const World* world);

inline void activate_measure_mode(u8 axis)
{
	deactivate_editor_modes();
	EdContext.measure_mode = true;
	EdContext.measure_axis = axis;
}

inline void check_selection_to_measure(const World* world)
{
	auto* GII = GlobalInputInfo::Get();

	auto pickray = cast_pickray(GSceneInfo.camera, GII->mouse_coords.x, GII->mouse_coords.y);
	auto test = world->Raycast(pickray);
	if(test.hit)
	{
		if(!EdContext.first_point_found || EdContext.second_point_found)
		{
			if(EdContext.second_point_found)
				EdContext.second_point_found = false;
			EdContext.first_point_found = true;
			EdContext.measure_from = point_from_detection(pickray, test);
		}
		else if(!EdContext.second_point_found)
		{
			EdContext.second_point_found = true;
			vec3 point = point_from_detection(pickray, test);
			if(EdContext.measure_axis == 0)
				EdContext.measure_to = point.x;
			else if(EdContext.measure_axis == 1)
				EdContext.measure_to = point.y;
			else if(EdContext.measure_axis == 2)
				EdContext.measure_to = point.z;
		}
	}
}

// ------------------------
// LOCATE COORDINATES MODE
// ------------------------
void activate_locate_coords_mode();
void check_selection_to_locate_coords(const World* world);

inline void activate_locate_coords_mode()
{
	deactivate_editor_modes();
	EdContext.locate_coords_mode = true;
	EdContext.locate_coords_found_point = false;
}

inline void check_selection_to_locate_coords(const World* world)
{
	auto* GII = GlobalInputInfo::Get();

	auto pickray = cast_pickray(GSceneInfo.camera, GII->mouse_coords.x, GII->mouse_coords.y);
	auto test = world->Raycast(pickray);
	if(test.hit)
	{
		EdContext.locate_coords_found_point = true;
		EdContext.locate_coords_position = point_from_detection(pickray, test);
	}
}

// -------------
// > MOVE TOOLS 
// -------------
inline void place_entity(World* world)
{
	/* Common function for move/rotate/scale entity tools.
	   Updates entity, tracks it state and updates world.
	   To be called at the end of entity modification operation. */

	EdContext.move_mode = false;
	EdContext.move_entity_by_arrows = false;
	EdContext.rotate_entity_with_mouse = false;
	EdContext.place_mode = false;
	EdContext.move_entity_by_arrows_ref_point = vec3(0);

	EdContext.selected_entity->Update();
	world->UpdateEntityWorldCells(EdContext.selected_entity);
	CL_recompute_collision_buffer_entities(GSceneInfo.player);
	EdContext.undo_stack.Track(EdContext.selected_entity);
}

inline RaycastTest test_ray_against_entity_support_plane(u16 move_axis, Entity* entity)
{
	// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
	// position. In the case of Y placement, we need to compute the plane considering the camera orientation.
	Triangle t1, t2;
	float plane_size = 500.0f;

	switch(EdContext.move_axis)
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
			auto camera = GSceneInfo.camera;
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

	auto ray = cast_pickray(GSceneInfo.camera, GII->mouse_coords.x, GII->mouse_coords.y);
	RaycastTest test;

	test = test_ray_against_triangle(ray, t1);
	if(!test.hit)
	{
		test = test_ray_against_triangle(ray, t2);
		if(!test.hit)
			std::cout << "warning: can't find plane to place entity!\n";
	}

	return test;
}

// --------------
// >> PLACE MODE
// --------------
inline void activate_place_mode(Entity* entity)
{
	deactivate_editor_modes();
	EdContext.place_mode = true;
	EdContext.selected_entity = entity;
	EdContext.undo_stack.Track(entity);
}

inline void select_entity_placing_with_mouse_move(Entity* entity, const World* world)
{
	auto* GII = GlobalInputInfo::Get();

	auto pickray = cast_pickray(GSceneInfo.camera, GII->mouse_coords.x, GII->mouse_coords.y);
	auto test = world->Raycast(pickray, entity);
	if(test.hit)
	{
		entity->position = point_from_detection(pickray, test);
		entity->Update();
	}
}


// -------------
// >> MOVE MODE
// -------------
inline void activate_move_mode(Entity* entity)
{
	deactivate_editor_modes();
	EdContext.move_mode = true;
	EdContext.move_axis = 0;
	EdContext.selected_entity = entity;
	EdContext.undo_stack.Track(entity);
}

inline void move_entity_with_mouse(Entity* entity)
{
	RaycastTest test = test_ray_against_entity_support_plane(EdContext.move_axis, entity);
	if(!test.hit)
		return;

	Ray ray = test.ray;
	vec3 pos = ray.origin + ray.direction * test.distance;

	// places entity accordingly
	switch(EdContext.move_axis)
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
void activate_move_entity_by_arrow(u8 move_axis);
void move_entity_by_arrows(Entity* entity);


inline void activate_move_entity_by_arrow(u8 move_axis)
{
	EdContext.move_axis = move_axis;
	EdContext.move_entity_by_arrows = true;
	auto test = test_ray_against_entity_support_plane(move_axis, EdContext.selected_entity);
	EdContext.move_entity_by_arrows_ref_point = point_from_detection(test.ray, test);
	EdContext.undo_stack.Track(EdContext.selected_entity);
}


inline void move_entity_by_arrows(Entity* entity)
{
	RaycastTest test = test_ray_against_entity_support_plane(EdContext.move_axis, entity);
	if(!test.hit)
		return;

	Ray ray = test.ray;
	vec3 pos = ray.origin + ray.direction * test.distance;

	// gets the offset from the mouse drag starting point, and not absolute position
	vec3 diff = pos - EdContext.move_entity_by_arrows_ref_point;

	// modifies entity position
	switch(EdContext.move_axis)
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

	EdContext.move_entity_by_arrows_ref_point = pos;

	entity->Update();

	// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
	update_entity_control_arrows(&EdContext.entity_panel);
	update_entity_rotation_gizmo(&EdContext.entity_panel);
}



// ----------------
// MOVE LIGHT TOOL
// ----------------
// @todo: This will DISAPPEAR after lights become entities!
//       We need to provide entity rights to lights too! revolution now!

void move_light_with_mouse(std::string type, int index, World* world);
void activate_move_light_mode(std::string type, int index);
void place_light(std::string type, int index);
void open_lights_panel(std::string type, int index, bool focus_tab); //fwd


inline void activate_move_light_mode(std::string type, int index)
{
	deactivate_editor_modes();
	EdContext.entity_panel.active = false;
	open_lights_panel(type, index, true);
	EdContext.move_mode = true;
	EdContext.selected_light = index;
	EdContext.selected_light_type = type;
}

inline void move_light_with_mouse(std::string type, int index, World* world)
{
	vec3 position;
	if(type == "point" && index > -1)
		position = world->point_lights[index]->position;
	else if(type == "spot" && index > -1)
		position = world->spot_lights[index]->position;
	else
		assert(false);

	auto* GII = GlobalInputInfo::Get();

	auto ray = cast_pickray(GSceneInfo.camera, GII->mouse_coords.x, GII->mouse_coords.y);

	// create a big plane for placing entity in the world with the mouse using raycast from camera to mouse
	// position. In the case of Y placement, we need to compute the plane considering the camera orientation.
	Triangle t1, t2;
	float plane_size = 500.0f;

	switch(EdContext.move_axis)
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
			auto camera = GSceneInfo.camera;
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

	test = test_ray_against_triangle(ray, t1);
	if(!test.hit)
	{
		test = test_ray_against_triangle(ray, t2);
		if(!test.hit)
		{
			std::cout << "warning: can't find plane to place light!\n";
			return;
		}
	}

	// places entity accordingly
	switch(EdContext.move_axis)
	{
		case 0: position.x = ray.origin.x + ray.direction.x * test.distance;
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

	if(type == "point" && index > -1)
		world->point_lights[index]->position = position;
	else if(type == "spot" && index > -1)
		world->spot_lights[index]->position = position;
	else
		assert(false);
}

inline void place_light()
{
	EdContext.move_mode = false;
	EdContext.selected_light = -1;
}


// ---------------------
// > ROTATE ENTITY TOOL
// ---------------------
void activate_rotate_entity_with_mouse(u8 move_axis);
float mouse_offset_to_angular_offset(float mouse_offset);
void rotate_entity_with_mouse(Entity* entity);

inline void activate_rotate_entity_with_mouse(u8 move_axis)
{
	auto* GII = GlobalInputInfo::Get();

	EdContext.move_axis = move_axis;
	EdContext.rotate_entity_with_mouse = true;
	EdContext.rotate_entity_with_mouse_mouse_coords_ref = vec2(
		GII->mouse_coords.x,
		GII->mouse_coords.y
	);
	EdContext.undo_stack.Track(EdContext.selected_entity);
}

inline float mouse_offset_to_angular_offset(float mouse_offset)
{
	// 360 degrees per 500 pixels of offset
	return mouse_offset * 360.f / 500.f;
}

inline void rotate_entity_with_mouse(Entity* entity)
{
	auto* GII = GlobalInputInfo::Get();
	auto mouse_coords = vec2(GII->mouse_coords.x, GII->mouse_coords.y);

	switch(EdContext.move_axis)
	{
		case 1: // X
		{
			float diff = mouse_coords.y - EdContext.rotate_entity_with_mouse_mouse_coords_ref.y;
			float angular_diff = mouse_offset_to_angular_offset(diff);
			entity->rotation.x += angular_diff;
			break;
		}
		case 2: // Y
		{
			float diff = mouse_coords.x - EdContext.rotate_entity_with_mouse_mouse_coords_ref.x;
			float angular_diff = mouse_offset_to_angular_offset(diff);
			entity->rotation.y += angular_diff;
			break;
		}
		case 3: // Z
		{
			float diff = mouse_coords.y - EdContext.rotate_entity_with_mouse_mouse_coords_ref.y;
			float angular_diff = mouse_offset_to_angular_offset(diff);
			entity->rotation.z += angular_diff;
			break;
		}
	}

	EdContext.rotate_entity_with_mouse_mouse_coords_ref = mouse_coords;
	entity->Update();

	// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
	update_entity_control_arrows(&EdContext.entity_panel);
	update_entity_rotation_gizmo(&EdContext.entity_panel);
}


// ------------------
// SCALE ENTITY TOOL
// ------------------
inline void scale_entity_with_mouse(Entity* entity)
{
	// NOT IMPLEMENTED
}


// -----------------------
// SELECT ENTITY AUX TOOL
// -----------------------
// used in entity panel to select other entity to attribute 1 to 1 relationships
inline void activate_select_entity_aux_tool(
	Entity** entity_slot,
	EdToolCallback callback = EdToolCallback_NoCallback,
	EdToolCallbackArgs args = EdToolCallbackArgs{}
)
{
	EdContext.select_entity_aux_mode = true;
	EdContext.select_entity_aux_mode_entity_slot = entity_slot;
	EdContext.select_entity_aux_mode_callback = callback;
	EdContext.select_entity_aux_mode_callback_args = args;
}


// -------------
// MISCELANEOUS
// -------------
// void check_for_asset_changes();
void render_aabb_boundaries(Entity* entity);

// void check_for_asset_changes()
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

inline void render_aabb_boundaries(Entity* entity)
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
