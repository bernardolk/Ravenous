#include "utils.h"
#include <algorithm>
#include "ctype.h"

void Tolower(string* Data)
{
	// could be rewritten without algorithm.h	
	std::transform(Data->begin(), Data->end(), Data->begin(), [](unsigned char Char) { return std::tolower(Char); });
}