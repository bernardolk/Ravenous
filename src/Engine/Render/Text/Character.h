#pragma once

struct Character
{
	uint texture_id;     // ID handle of the glyph texture
	uint advance;        // Offset to advance to next glyph
	glm::ivec2 size;    // Size of glyph
	glm::ivec2 bearing; // Offset from baseline to left/top of glyph
};
