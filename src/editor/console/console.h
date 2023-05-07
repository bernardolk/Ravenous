#pragma once
#include "engine/core/core.h"

struct InputFlags;

void HandleConsoleInput(InputFlags flags, Player* & player, T_World* world, Camera* camera);
void ExecuteCommand(const string& buffer_line, Player* & player, T_World* world, Camera* camera);
void CheckLetterKeyPresses(InputFlags flags);
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

struct GlobalConsoleState
{
	u16 buffer_capacity = 20;
	constexpr static u16 max_chars = 50;
	char** buffers;
	u16 b_ind = 0;
	u16 current_buffer_size = 0;
	u16 buffer_size_incr = 1;
	char scratch_buffer[max_chars];
	u16 c_ind = 0;
};

//todo: refactor global
inline GlobalConsoleState Console;