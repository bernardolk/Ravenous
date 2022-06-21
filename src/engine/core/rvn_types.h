#pragma once
#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;

// globals
const static float PI = 3.141592;

extern const float VEC_COMPARE_PRECISION;
extern const float MAX_FLOAT;
extern const float MIN_FLOAT;

extern const vec3 UNIT_X;
extern const vec3 UNIT_Y;
extern const vec3 UNIT_Z;

inline bool is_equal(vec2 vec1, vec2 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   
   return x_diff < VEC_COMPARE_PRECISION && y_diff < VEC_COMPARE_PRECISION;
}

inline bool is_equal(vec3 vec1, vec3 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   float z_diff = abs(vec1.z - vec2.z);

   return x_diff < VEC_COMPARE_PRECISION 
      && y_diff < VEC_COMPARE_PRECISION 
      && z_diff < VEC_COMPARE_PRECISION;
}

// TYPE GUARANTEES

inline bool operator==(const vec3& lhs, const vec3& rhs)
{
   return is_equal(lhs, rhs);
}

inline bool operator==(const vec2& lhs, const vec2& rhs)
{
   return is_equal(lhs, rhs);
}

inline bool operator!=(const vec3& lhs, const vec3& rhs)
{
   return !is_equal(lhs, rhs);
}

inline bool operator!=(const vec2& lhs, const vec2& rhs)
{
   return !is_equal(lhs, rhs);
}


// VECTOR OPERATIONS

inline vec3 cross(vec3 a, vec3 b, vec3 c)
{
   return glm::cross(glm::cross(a, b), c);
}


// VECTOR CONVERSIONS

inline vec3 toVec3(vec4 vec)
{
   return vec3(vec.x, vec.y, vec.z);
}

