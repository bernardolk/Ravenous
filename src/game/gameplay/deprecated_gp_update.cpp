
/*	  BIG @TODO
      player collider update is a mess, I've decoupled it from the main player update call, so now
      we must update (translate) the player's AABB every time its position changes.
      Lets fix this please.  
*/

// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions)
// {
//    /* Does a circular array of raycasts according to parameters.
//       The circle will be considered to be 'touching the ground', hence limits for stepping up or down are applied
//       from the "center" arg y component.

//       center: circle's center
//       radius: circle radius
//       orientation0: reference direction for first ray
//       angle: angle span from 0 to 360
//       subdivisions: controls how many rays to shoot
//    */

//       bool hit_terrain = false;
//       vec3 orientation = normalize(to3d_xz(orientation0));
//       float delta_angle = angle / subdivisions;
//       float current_angle = 0;
//       while(current_angle <= angle)
//       {
//          orientation = rotate(orientation, glm::radians(delta_angle), UNIT_Y);
//          vec3 ray_origin = center + orientation * radius;
//          // moves ray up a bit
//          ray_origin.y = center.y + 1;

//          vec3 color = COLOR_RED_1;
//          if(current_angle <= angle / 2)
//             color = COLOR_RED_2;

//          bool hit = GP_do_vtrace_for_terrain(ray_origin, center.y, color);
//          hit_terrain = hit_terrain || hit;

//          current_angle += delta_angle;
//       }

//       return hit_terrain;
// }


// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color = COLOR_RED_1)
// {
//    auto vtrace_ray = Ray{ vtrace_origin, -UNIT_Y };
//    RaycastTest vtrace = test_ray_against_scene(vtrace_ray);
//    if(vtrace.hit)
//    {
//       auto hitpoint = point_from_detection(vtrace_ray, vtrace);
//       // draw arrow
//       ImDraw::add_line (IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, vtrace_origin, debug_color);
//       ImDraw::add_point(IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, debug_color);

//       float delta_y = abs(terrain_baseline_height - hitpoint.y);
//       if(delta_y <= PLAYER_STEPOVER_LIMIT)
//       {
//          // disable collision with potential floor / terrain
//          CL_Ignore_Colliders.add(vtrace.entity);
//          return true;
//       }
//    }

//    return false;
// }


// -------------------
// > ACTION
// -------------------

// void GP_CheckTriggerInteraction(Player* player, T_World* world)
// {
	/**
	For(world->interactables.size())
	{
		auto interactable = world->interactables[i];

		//@todo: do a cylinder vs cylinder or cylinder vs aabb test here
		CollisionMesh trigger_collider = interactable->GetTriggerCollider();
		GJK_Result gjk_test = CL_RunGjk(&player->collider, &trigger_collider);
		if (gjk_test.collision)
		{
			Rvn::PrintDynamic("Trigger Interaction", 1000);

			switch (interactable->type)
			{
				case EntityType_Checkpoint:
				{
					player->SetCheckpoint(interactable);
					break;
				}
				case EntityType_TimerTrigger:
				{
					GameState.StartTimer(interactable);
					break;
				}
			}
		}
	}
	*/
// }

// -------------------
// > LEDGE GRABBING
// -------------------
// void GP_CheckPlayerGrabbedLedge(Player* player, T_World* world)
// {
// 	Ledge ledge = CL_PerformLedgeDetection(player, world);
// 	if (ledge.empty)
// 		return;
// 	vec3 position = CL_GetFinalPositionLedgeVaulting(player, ledge);
//
// 	PlayerStateChangeArgs args;
// 	args.ledge = ledge;
// 	args.final_position = position;
// 	player->ChangeStateTo(PlayerState::Vaulting, args);
// }

// void GP_check_player_grabbed_ledge(Player* player)
// {
//    // ledge grab y tollerance
//    const float y_tol = 0.1;
//    // half the ledge grab semicircle region angle, in degrees 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->top().y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);


//    for(int i = 0; i < RVN::entity_buffer->size; i++)
//    {
//       Entity* entity = RVN::entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
//       {
//          float edge_y = entity->position.y + entity->get_height();
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->position.x, 
//             player->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             ImDraw::add_mesh(IMHASH, player, future_pos);
//             if(CL_test_in_mock_position(player, future_pos))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PlayerState::Grabbing, ps_args);
//             return;
//          }
//       }

//       else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
//       {
//          float edge_y = entity->position.y;
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->position.x, 
//             player->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          // player is not facing slope's inclined face
//          if(get_slope_normal(entity) != test.normal_vec)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             ImDraw::add_mesh(IMHASH, player, future_pos);
//             if(CL_test_in_mock_position(player, future_pos, entity))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PlayerState::Grabbing, ps_args);
//             return;
//          }
//       }
//    }
// }


// -------------------
// > VAULTING
// -------------------

// bool GP_check_player_vaulting(Player* player)
// {
//    // action cone half theta 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->position.y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);

//    for(int i = 0; i < RVN::entity_buffer->size; i++)
//    {
//       Entity* entity = RVN::entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
//          continue;

//       float rel_height = (entity->position.y + entity->get_height()) - player->feet().y;

//       // short platforms should be ignored since we will use navigation meshes that include them smoothly with a nav ramp 
//       // and therefore going over them do not count as 'vaulting moves'
//       if(rel_height < 0.3) // also makes sure we only get positive rel heights
//          continue;

//       if(rel_height >= player->half_height * 2)
//          continue;

//       auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//       auto test = CL_circle_vs_square(
//          player->position.x,
//          player->position.z,
//          player->radius + dr,
//          x0, x1, z0, z1
//       );

//       if(!test.is_collided)
//          continue;

//       float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//       float min_theta = 180 - s_theta;
//       float max_theta = 180 + s_theta;
//       if(min_theta <= theta && theta <= max_theta)
//       {
//          // checks if area above ledge is free for standing
//          vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//          // ImDraw::add_mesh(IMHASH, player, future_pos);
//          if(CL_test_in_mock_position(player, future_pos))
//          {
//             RVN::print_dynamic("Vaulting failed.");
//             continue;
//          }

//          PlayerStateChangeArgs ps_args;
//          ps_args.entity = entity;
//          // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//          ps_args.final_position = future_pos;
//          ps_args.penetration = dr - test.overlap;

//          GP_change_player_state(player, PlayerState::Vaulting, ps_args);
//          return true;
//       }
//    }
//    return false;
// }


// void GP_check_player_events(Player* player)
// {
//    // Player death
//    if(player->lives <= 0)
//    {
//       RVN::rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
//       player->die();
//       return;
//    }
// }
