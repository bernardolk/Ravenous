#include <limits>
#include <map>
#include <string>
#include <engine/core/rvn_types.h>

// VECTOR COMPARISON
extern const float VecComparePrecision = 0.00001f;
extern const float MaxFloat = std::numeric_limits<float>::max();
extern const float MinFloat = -MaxFloat;

// AXIS
extern const vec3 UnitX = vec3(1, 0, 0);
extern const vec3 UnitY = vec3(0, 1, 0);
extern const vec3 UnitZ = vec3(0, 0, 1);
