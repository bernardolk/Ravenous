#include <engine/core/types.h>
#include <engine/collision/simplex.h>

void RSimplex::PushFront(vec3 point)
{
	this->points[3] = this->points[2];
	this->points[2] = this->points[1];
	this->points[1] = this->points[0];
	this->points[0] = point;

	this->p_size++;
	assert(this->p_size <= 4);
}

vec3& RSimplex::operator[](u32 i) { return this->points[i]; }
u32 RSimplex::size() const { return this->p_size; }
