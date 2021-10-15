// ----------------------------------------
// > COLLISION STATE RESOLVER
// ----------------------------------------

void CL_resolve_collision(EntitiesCollision collision, Player* player)
{
   // the point of this is to not trigger health check 
   // when player hits a wall while falling
   bool trigger_check_was_player_hurt = false;

   // if collided, unset mid-air controls
   player->jumping_upwards = false;
   switch(collision.collision_outcome)
   {
      case JUMP_SUCCESS:
      {
         trigger_check_was_player_hurt = true;
         std::cout << "LANDED" << "\n";
         // move player to surface, stop player and set him to standing
         player->standing_entity_ptr            = collision.collided_entity_ptr;
         auto height_check                      = CL_get_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
         player->entity_ptr->position.y         = height_check.overlap + player->half_height; 
         player->entity_ptr->velocity           = vec3(0,0,0);
         player->player_state                   = PLAYER_STATE_STANDING;
         // conditional animation: if falling from jump, land, else, land from fall
         if(player->half_height < P_HALF_HEIGHT)
            player->anim_state                  = P_ANIM_LANDING;
         else
            player->anim_state                  = P_ANIM_LANDING_FALL;
         break;
      }


      case JUMP_FACE_FLAT:
      {
         std::cout << "JUMP FACE FLAT" << "\n";
         // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
         player->entity_ptr->position           += vec3(collision.normal_vec.x, 0, collision.normal_vec.y) * collision.overlap;

         // make player slide through the tangent of platform
         auto tangent_vec                       = vec2(collision.normal_vec.y, collision.normal_vec.x);      
         auto v_2d                              = vec2(player->entity_ptr->velocity.x, player->entity_ptr->velocity.z);
         auto project                           = (glm::dot(v_2d, tangent_vec)/glm::length2(tangent_vec))*tangent_vec;

         player->entity_ptr->velocity.x         = project.x;
         player->entity_ptr->velocity.z         = project.y; 
               
         if(player->player_state == PLAYER_STATE_JUMPING)
         {
            player->player_state                = PLAYER_STATE_FALLING;
            player->entity_ptr->velocity.y      = 0;
         }
         break;
      }


      case JUMP_SLIDE:
      {
         trigger_check_was_player_hurt          = true;
         GP_make_player_slide(player, collision.collided_entity_ptr);
         break;
      }


      case JUMP_SLIDE_HIGH_INCLINATION:
      {
         trigger_check_was_player_hurt          = true;
         GP_make_player_slide(player, collision.collided_entity_ptr, true);
         break;
      }


      case JUMP_CEILING:
      {
         std::cout << "HIT CEILING" << "\n";
         player->entity_ptr->position.y         -= collision.overlap + COLLISION_EPSILON; 
         player->player_state                   = PLAYER_STATE_FALLING;
         player->entity_ptr->velocity.y         = 0;
         break; 
      }


      case STEPPED_SLOPE:
      {
         // cout << "PLAYER STEPPED INTO SLOPE \n";
         player->standing_entity_ptr            = collision.collided_entity_ptr;
         player->entity_ptr->position.y         += collision.overlap;
         break;
      }


      case BLOCKED_BY_WALL:
      {
         // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
         player->entity_ptr->position           += vec3(collision.normal_vec.x, 0, collision.normal_vec.y) * collision.overlap;
         break;
      }
   }

   // hurts player if necessary
   if(trigger_check_was_player_hurt) player->maybe_hurt_from_fall();
}