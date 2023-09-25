#pragma once

struct RCharacter
{
	uint TextureID;    // ID handle of the glyph texture
	uint Advance;       // Offset to advance to next glyph
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
};
