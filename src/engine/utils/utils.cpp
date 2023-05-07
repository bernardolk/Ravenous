#include "utils.h"
#include <algorithm>
#include "ctype.h"

void Tolower(string* data)
{
	// could be rewritten without algorithm.h	
	std::transform(data->begin(), data->end(), data->begin(), [](unsigned char c) { return std::tolower(c); });
}

int Test()
{
	vec3 my_vec{2.f};
	int a = my_vec.x;
	return a;
}
