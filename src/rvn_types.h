#include <limits>

// TYPE DEFINITIONS
typedef int i16;
typedef long int i32;
typedef long long int i64;
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long u64;
const float PI = 3.141592;
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;
typedef std::string string;

// VECTOR COMPARISON
float VEC_COMPARE_PRECISION = 0.00001f;
const float MAX_FLOAT = std::numeric_limits<float>::max();
const float MIN_FLOAT = std::numeric_limits<float>::min();

// AXIS
namespace vec3
{
   const vec3 unit_x = vec3(1,0,0);
   const vec3 unit_y = vec3(1,0,0);
   const vec3 unit_z = vec3(1,0,0);
};


inline
bool is_equal(vec2 vec1, vec2 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   
   return x_diff < VEC_COMPARE_PRECISION && y_diff < VEC_COMPARE_PRECISION;
}

inline
bool is_equal(vec3 vec1, vec3 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   float z_diff = abs(vec1.z - vec2.z);

   return x_diff < VEC_COMPARE_PRECISION 
      && y_diff < VEC_COMPARE_PRECISION 
      && z_diff < VEC_COMPARE_PRECISION;
}

// TYPE GUARANTEES
inline
bool operator==(const vec3& lhs, const vec3& rhs)
{
   return is_equal(lhs, rhs);
}

inline
bool operator==(const vec2& lhs, const vec2& rhs)
{
   return is_equal(lhs, rhs);
}

inline
bool operator!=(const vec3& lhs, const vec3& rhs)
{
   return !is_equal(lhs, rhs);
}

inline
bool operator!=(const vec2& lhs, const vec2& rhs)
{
   return !is_equal(lhs, rhs);
}


// VECTOR OPERATIONS

inline
vec3 cross(vec3 a, vec3 b, vec3 c)
{
   return glm::cross(glm::cross(a, b), c);
}