#include <engine/core/types.h>
#include <engine/collision/simplex.h>

void RSimplex::PushFront(vec3 Point)
{
	this->Points[3] = this->Points[2];
	this->Points[2] = this->Points[1];
	this->Points[1] = this->Points[0];
	this->Points[0] = Point;

	this->PSize++;
	assert(this->PSize <= 4);
}

vec3& RSimplex::operator[](uint i) { return this->Points[i]; }
uint RSimplex::size() const { return this->PSize; }
