#pragma once
#include "engine/core/core.h"

struct RInputFlags;

void HandleConsoleInput(RInputFlags flags, EPlayer* & player, RWorld* world, RCamera* camera);
void ExecuteCommand(const string& BufferLine, EPlayer* & player, RWorld* world, RCamera* camera);
void CheckLetterKeyPresses(RInputFlags flags);
void ClearConsoleStringBuffer();
void RenderConsole();
void StartConsoleMode();
void QuitConsoleMode();
void MoveToPreviousBuffer();
void MoveToNextBuffer();
string CommitBuffer();
void InitializeConsoleBuffers();
void CopyBufferToScratchBuffer();
void ClearScratchBuffer();

struct RGlobalConsoleState
{
	static RGlobalConsoleState* Get()
	{
		static RGlobalConsoleState Instance{};
		return &Instance;
	}

	uint16 buffer_capacity = 20;
	constexpr static uint16 max_chars = 50;
	char** buffers;
	uint16 b_ind = 0;
	uint16 current_buffer_size = 0;
	uint16 buffer_size_incr = 1;
	char scratch_buffer[max_chars];
	uint16 c_ind = 0;
};
