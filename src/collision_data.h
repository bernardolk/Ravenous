const float COLLISION_EPSILON = 0.0001f;

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS                = 0,  // player fell succesfully into another platform
// JUMP_FAIL                   = 1,  // player fell into the edge of another platform
   JUMP_FACE_FLAT              = 2,  // player fell with face flat on wall
   JUMP_SLIDE                  = 3,  // player is sliding
   JUMP_SLIDE_HIGH_INCLINATION = 4,  // player is sliding but cant move
   JUMP_CEILING                = 5,  // player hit the ceiling with his head
   STEPPED_SLOPE               = 6,  // player entered a low inclination slope
   BLOCKED_BY_WALL             = 7,  // player hit a wall
   NO_OUTCOME                  = 999
};

struct CollisionData {
   bool is_collided            = false;
   Entity* collided_entity_ptr = NULL;
   float overlap               = 0;
   vec2 normal_vec        = vec2(0,0);
   CollisionOutcomeEnum collision_outcome;
};

struct Collision {
   bool is_collided  = false;
   float overlap     = 0;
   vec2 normal_vec   = vec2(0.0f);
   bool is_inside    = false;
};