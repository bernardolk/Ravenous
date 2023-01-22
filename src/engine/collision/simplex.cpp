#include <engine/core/rvn_types.h>
#include <engine/collision/simplex.h>

void Simplex::PushFront(vec3 point)
{
	this->points[3] = this->points[2];
	this->points[2] = this->points[1];
	this->points[1] = this->points[0];
	this->points[0] = point;

	this->p_size++;
	assert(this->p_size <= 4);
}

vec3& Simplex::operator[](u32 i) { return this->points[i]; }
u32 Simplex::size() const { return this->p_size; }
