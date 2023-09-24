#pragma once

#include "Deps.h"
#include "Constants.h"

/** Primitive types */
using uint8 = unsigned char;
using uint16 = unsigned short int;
using uint = unsigned int;
using uint64 = unsigned long long;

using int16 = short int;
using int64 = long int;

#ifndef OS_WINDOWS_INCLUDED
using byte = unsigned char;
#endif

/** GLM type aliases */
using vec4 = glm::vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using mat4 = glm::mat4;

template<typename T>
using vector = std::vector<T>;

template<typename T, typename T2>
using map = std::map<T, T2>;

using GLenum = unsigned int;

using string = std::string;

using Flags = uint;

using TypeID = uint;
using TraitID = uint;

inline const vec3 UnitX = vec3(1, 0, 0);
inline const vec3 UnitY = vec3(0, 1, 0);
inline const vec3 UnitZ = vec3(0, 0, 1);

using std::to_string;

static constexpr float FloatEpsilon = 1.192092896e-07F;

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

inline bool IsEqual(float x, float y)
{
	constexpr int ulp = 2;
	return fabs(x - y) <= Epsilon * fabs(x + y) * ulp || fabs(x - y) < MinFloat;
}

inline bool IsEqual(double x, double y)
{
	constexpr int ulp = 2;
	return fabs(x - y) <= EpsilonDouble * fabs(x + y) * ulp || fabs(x - y) < MinDouble;
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
	return cross(cross(a, b), c);
}

inline vec3 ToVec3(vec4 vec)
{
	return vec3(vec.x, vec.y, vec.z);
}

template<typename T>
struct Iterator
{
	T* it;

	uint it_count;
	const uint64 count;

	explicit Iterator(T* start, const uint64 count) : count(count)
	{
		it = start;
		it_count = 0;
	};

	T* operator()()
	{
		if (it_count < count)
		{
			return it + it_count++;;
		}

		return nullptr;
	};
};

template<typename T_Key, typename T_Val>
T_Val* Find(map<T_Key, T_Val>& map, T_Key key)
{
	auto find = map.find(key);
	if (find == map.end())
		return nullptr;

	return &find->second;
}

template<typename T_Key, typename T_Val>
const T_Val* Find(const map<T_Key, T_Val>& map, T_Key key)
{
	auto find = map.find(key);
	if (find == map.end())
		return nullptr;

	return &find->second;
}

/** Basic iterable array data structure */
template<typename T, uint64 Size>
struct Array
{

private:
	T data[Size];
	uint64 count = 0;

public:
	Array() = default;
	
	Array(T* data, uint64 count)
	{
		for (uint64 i = 0; i < count; i++)
			this->data[i] = data[i];
		
		this->count = count;
	}

	explicit Array(T& default_obj)
	{
		for (uint i = 0; i < Size; i++)
			this->data[i] = default_obj;
	}

	uint Num() { return count; }

	Iterator<T> GetIterator()
	{
		Iterator<T> it(&data[0], Size);
		return it;
	}

	T* GetAt(int i)
	{
		if (i >= count)
			return nullptr;
		
		return &data[i];
	}
	
	T* Add(const T& instance)
	{
		if (count < Size)
		{
			data[count] = instance;
			return &data[count++];
		}

		return nullptr;
	};

	T* AddNew()
	{
		if (count < Size)
		{
			data[count] = T();
			return &data[count++];
		}

		return nullptr;
	};

	unsigned int GetCount() const
	{
		return count;
	}

	bool Contains(T& item)
	{
		for (uint i = 0; i < count; i++)
		{
			if (data[i] == item)
				return true;
		}
		return false;
	}

	T* begin() { return &data[0]; }
	T* end() { return &data[count]; }
	Array Copy() { return Array(&data[0], count); }
	
	using Lambda = bool (*)(T&);
	bool Eval(Lambda f)
	{
		for (uint64 i = 0; i < count; i++)
		{
			if (f(&data[i]))
			{
				return true;
			}
		}
		return false;
	}
};

template<typename T, uint8 Order, uint16 Dimension>
struct Matrix
{
	static constexpr uint16 dimension = Dimension;
	static constexpr uint8 order = Order;
};

template<typename T, uint16 Dimension>
struct Matrix<T, 3, Dimension>
{
	Array<Array<Array<T, Dimension>, Dimension>, Dimension> data{};

	T* GetAt(uint8 i, uint8 j, uint8 k)
	{
		return data.GetAt[i].GetAt[j].GetAt[k];
	}

	T* AddAt(const T& instance, uint8 i, uint8 j, uint8 k)
	{
		if (i < Dimension && j < Dimension && k < Dimension)
		{
			T* obj = data.GetAt(i)->GetAt(j)->GetAt(k);
			*obj = instance;
			return obj;
		}
		else
		{
			// Matrix overflow
			assert(false);
		}
		
		return nullptr;
	};

	Iterator<T> GetIterator()
	{
		Iterator<T> it(&data, Dimension * Dimension * Dimension);
		return it;
	}
};