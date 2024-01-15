#pragma once
#include "Engine/Core/Core.h"

// ===========================
// RView
// ===========================
// View into an element inside a dynamic container (such as std::vector)
// This exists because it is not safe to store ptr to vector items since the vector can resize and rellocate all items at any point
template<typename T>
struct RView
{
	RView(vector<T>* InCollection, T* Item) : Collection(InCollection)
	{
		Offset = Item - &(*InCollection)[0];
		assert(Offset >= 0);
	}

	RView() = default;

	T* Get() const
	{
		return IsValid() ? &(*Collection)[Offset] : nullptr;
	}

	T* operator ->()
	{
		return Get();	
	}

	T* operator*()
	{
		return Get();
	}

	T* operator ->() const
	{
		return Get();	
	}

	bool IsValid() const
	{
		return Collection && Offset >= 0;
	}

private:
		vector<T>* Collection = nullptr;
		int Offset = -1;
};
