#include <limits>
#include <map>
#include <string>
#include <engine/core/rvn_types.h>

// VECTOR COMPARISON
extern const float VEC_COMPARE_PRECISION = 0.00001f;
extern const float MAX_FLOAT = std::numeric_limits<float>::max();
extern const float MIN_FLOAT = -MAX_FLOAT;

// AXIS
extern const vec3 UNIT_X = vec3(1, 0, 0);
extern const vec3 UNIT_Y = vec3(0, 1, 0);
extern const vec3 UNIT_Z = vec3(0, 0, 1);
