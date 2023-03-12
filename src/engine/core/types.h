#pragma once

/** STD INCLUDES */
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <stack>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>


/** GLM TYPE INCLUDES */
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>


/** Primitive types */
using u8 = unsigned char;
using u16 = unsigned short int;
using u32 = unsigned int;
using u64 = unsigned long long;

/** GLM type aliases */
using vec4 = glm::vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using mat4 = glm::mat4;

template<typename T>
using vector = std::vector<T>;

using GLenum = unsigned int;

using string = std::string;

using TypeID = unsigned int;
using TraitID = unsigned int;

extern const float VecComparePrecision;
extern const float MaxFloat;
extern const float MinFloat;

inline const vec3 UnitX = vec3(1, 0, 0);
inline const vec3 UnitY = vec3(0, 1, 0);
inline const vec3 UnitZ = vec3(0, 0, 1);

inline bool IsEqual(vec2 vec1, vec2 vec2)
{
	float x_diff = abs(vec1.x - vec2.x);
	float y_diff = abs(vec1.y - vec2.y);

	return x_diff < VecComparePrecision && y_diff < VecComparePrecision;
}

inline bool IsEqual(vec3 vec1, vec3 vec2)
{
	float x_diff = abs(vec1.x - vec2.x);
	float y_diff = abs(vec1.y - vec2.y);
	float z_diff = abs(vec1.z - vec2.z);

	return x_diff < VecComparePrecision
	&& y_diff < VecComparePrecision
	&& z_diff < VecComparePrecision;
}

// TYPE GUARANTEES

inline bool operator==(const vec3& lhs, const vec3& rhs)
{
	return IsEqual(lhs, rhs);
}

inline bool operator==(const vec2& lhs, const vec2& rhs)
{
	return IsEqual(lhs, rhs);
}

inline bool operator!=(const vec3& lhs, const vec3& rhs)
{
	return !IsEqual(lhs, rhs);
}

inline bool operator!=(const vec2& lhs, const vec2& rhs)
{
	return !IsEqual(lhs, rhs);
}

inline vec3 Cross(vec3 a, vec3 b, vec3 c)
{
	return glm::cross(glm::cross(a, b), c);
}

inline vec3 ToVec3(vec4 vec)
{
	return vec3(vec.x, vec.y, vec.z);
}

template<typename T>
struct Iterator
{
	T* it;

	unsigned int it_count;
	const unsigned int count;

	explicit Iterator(T* start, const unsigned int&& count) : count(count)
	{
		it = start;
		it_count = 0;
	};

	T* operator()()
	{
		if(it_count < count)
		{
			return it + it_count++;;
		}

		return nullptr;
	};
};

template<typename T_Key, typename T_Val>
T_Val* Find(std::map<T_Key, T_Val>& map, T_Key key)
{
	auto find = map.find(key);
	if (find == map.end())
		return nullptr;

	return &find->second;
}

template<typename T_Key, typename T_Val>
const T_Val* Find(const std::map<T_Key, T_Val>& map, T_Key key)
{
	auto find = map.find(key);
	if (find == map.end())
		return nullptr;

	return &find->second;
}


template<typename T, unsigned int Size>
struct T_Array
{

public:
	T array[Size];
	unsigned int count = 0;

public:
	T* Add(const T instance)
	{
		if(count < Size)
		{
			array[count] = instance;
			auto* tmp = &array[count];
			count++;
			return tmp;
		}
#ifdef T_ARRAY_OVERFLOW_IS_FATAL
		else
		{
			assert(false);
		}
#endif

		return nullptr;
	};

	T* AddAt(const T& instance, unsigned int i)
	{
		if(i < Size)
		{
			array[i] = instance;
			return &array[i];
		}
#ifdef T_ARRAY_OVERFLOW_IS_FATAL
		else
		{
			assert(false);
		}
#endif

		return nullptr;
	};

	T* GetNextSlot()
	{
		if(count < Size)
		{
			return &(array[count++]);
		}
#ifdef T_ARRAY_OVERFLOW_IS_FATAL
		else
		{
			assert(false);
		}
#endif

		return nullptr;
	}

	unsigned int GetCount() const
	{
		return count;
	}

	T* begin()
	{
		return &array[0];
	}

	T* end()
	{
		return &array[count];
	}
};