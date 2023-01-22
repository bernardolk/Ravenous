#pragma once

#include "engine/core/core.h"

struct EntityPool
{

	Entity* pool = nullptr;
	const int size;
	int count = 0;

	explicit EntityPool(const int size) :
		size(size) { }

	void Init();

	// @TODO: Refactor, this is stupid
	[[nodiscard]] Entity* GetNext() const;

	//@TODO: Refactor, this is stupid
	void FreeSlot(const Entity* entity) const;
};
