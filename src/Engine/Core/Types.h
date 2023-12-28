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

using RTypeID = uint;
using RTraitID = uint;
using RUUID = uint64;

inline const vec3 UnitX = vec3(1, 0, 0);
inline const vec3 UnitY = vec3(0, 1, 0);
inline const vec3 UnitZ = vec3(0, 0, 1);

using std::to_string;

static constexpr float FloatEpsilon = 1.192092896e-07F;

inline bool IsEqual(vec2 Vec1, vec2 Vec2)
{
	float XDiff = abs(Vec1.x - Vec2.x);
	float YDiff = abs(Vec1.y - Vec2.y);

	return XDiff < VecComparePrecision && YDiff < VecComparePrecision;
}

inline bool IsEqual(vec3 Vec1, vec3 Vec2)
{
	float XDiff = abs(Vec1.x - Vec2.x);
	float YDiff = abs(Vec1.y - Vec2.y);
	float ZDiff = abs(Vec1.z - Vec2.z);

	return XDiff < VecComparePrecision
	&& YDiff < VecComparePrecision
	&& ZDiff < VecComparePrecision;
}

inline bool IsEqual(float X, float Y)
{
	constexpr int Ulp = 2;
	return fabs(X - Y) <= Epsilon * fabs(X + Y) * Ulp || fabs(X - Y) < MinFloat;
}

inline bool IsEqual(double X, double Y)
{
	constexpr int Ulp = 2;
	return fabs(X - Y) <= EpsilonDouble * fabs(X + Y) * Ulp || fabs(X - Y) < MinDouble;
}


// TYPE GUARANTEES

inline bool operator==(const vec3& Lhs, const vec3& Rhs)
{
	return IsEqual(Lhs, Rhs);
}

inline bool operator==(const vec2& Lhs, const vec2& Rhs)
{
	return IsEqual(Lhs, Rhs);
}

inline bool operator!=(const vec3& Lhs, const vec3& Rhs)
{
	return !IsEqual(Lhs, Rhs);
}

inline bool operator!=(const vec2& Lhs, const vec2& Rhs)
{
	return !IsEqual(Lhs, Rhs);
}

inline vec3 Cross(vec3 A, vec3 B, vec3 C)
{
	return cross(cross(A, B), C);
}

inline vec3 ToVec3(vec4 Vector)
{
	return vec3(Vector.x, Vector.y, Vector.z);
}

template<typename T>
struct TIterator
{
	T* Value;

	uint ItCount;
	const uint64 Count;

	explicit TIterator(T* Start, const uint64 Count) :
		Count(Count)
	{
		Value = Start;
		ItCount = 0;
	};

	T* operator()()
	{
		if (ItCount < Count)
		{
			return Value + ItCount++;;
		}

		return nullptr;
	};
};

template<typename TKey, typename TVal>
TVal* Find(map<TKey, TVal>& Map, TKey& Key)
{
	auto Find = Map.find(Key);
	if (Find == Map.end())
		return nullptr;

	return &Find->second;
}

template<typename TKey, typename TVal>
TVal* Find(map<TKey, TVal>& Map, const TKey&& Key)
{
	auto Find = Map.find(Key);
	if (Find == Map.end())
		return nullptr;

	return &Find->second;
}

template<typename TVal>
TVal* Find(map<string, TVal>& Map, const char* Key)
{
	auto Find = Map.find(Key);
	if (Find == Map.end())
		return nullptr;

	return &Find->second;
}

template<typename TVal>
TVal* Find(map<string, TVal>& Map, const string& Key)
{
	auto Find = Map.find(Key);
	if (Find == Map.end())
		return nullptr;

	return &Find->second;
}

template<typename TKey, typename TVal>
const TVal* Find(const map<TKey, TVal>& Map, TKey& Key)
{
	auto Find = Map.find(Key);
	if (Find == Map.end())
		return nullptr;

	return &Find->second;
}

/** Basic iterable array data structure */
template<typename T, uint64 Size>
struct Array
{

private:
	T Data[Size];
	uint64 Count = 0;

public:
	Array() = default;

	Array(T* Data, uint64 Count)
	{
		for (uint64 i = 0; i < Count; i++)
			this->Data[i] = Data[i];

		this->Count = Count;
	}

	explicit Array(T& DefaultObj)
	{
		for (uint i = 0; i < Size; i++)
			this->Data[i] = DefaultObj;
	}

	uint Num() { return Count; }

	TIterator<T> GetIterator()
	{
		TIterator<T> Iterator(&Data[0], Size);
		return Iterator;
	}

	T* GetAt(int i)
	{
		if (i >= Count)
			return nullptr;

		return &Data[i];
	}

	T* Add(const T& Instance)
	{
		if (Count < Size)
		{
			Data[Count] = Instance;
			return &Data[Count++];
		}

		return nullptr;
	};

	T* AddNew()
	{
		if (Count < Size)
		{
			Data[Count] = T();
			return &Data[Count++];
		}

		return nullptr;
	};

	unsigned int GetCount() const
	{
		return Count;
	}

	bool Contains(T& Item)
	{
		for (uint i = 0; i < Count; i++)
		{
			if (Data[i] == Item)
				return true;
		}
		return false;
	}

	T* begin() { return &Data[0]; }
	T* end() { return &Data[Count]; }
	Array Copy() { return Array(&Data[0], Count); }

	using Lambda = bool (*)(T&);
	bool Eval(Lambda Func)
	{
		for (uint64 i = 0; i < Count; i++)
		{
			if (Func(&Data[i]))
			{
				return true;
			}
		}
		return false;
	}
};

template<typename T, uint8 TOrder, uint16 TDimension>
struct Matrix
{
	static constexpr uint16 Dimension = TDimension;
	static constexpr uint8 Order = TOrder;
};

template<typename T, uint16 Dimension>
struct Matrix<T, 3, Dimension>
{
	Array<Array<Array<T, Dimension>, Dimension>, Dimension> Data;

	Matrix(){ Data = Array<Array<Array<T, Dimension>, Dimension>, Dimension>{}; }

	T* GetAt(uint8 i, uint8 j, uint8 k)
	{
		return Data.GetAt[i].GetAt[j].GetAt[k];
	}

	T* AddAt(const T& Instance, uint8 i, uint8 j, uint8 k)
	{
		if (i < Dimension && j < Dimension && k < Dimension)
		{
			T* Obj = Data.GetAt(i)->GetAt(j)->GetAt(k);
			*Obj = Instance;
			return Obj;
		}
		else
		{
			// Matrix overflow
			assert(false);
		}

		return nullptr;
	};

	TIterator<T> GetIterator()
	{
		TIterator<T> Iterator(&Data, Dimension * Dimension * Dimension);
		return Iterator;
	}
};
