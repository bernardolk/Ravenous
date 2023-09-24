#pragma once

#include "engine/core/core.h"
#include "engine/io/InputPhase.h"

constexpr static int MaxInputRecordings = 20;
const std::string RecordingsFilenamePrefix = "rec-";
const std::string RecordingsFilenameExtension = ".txt";

struct RecordedInput
{
	std::vector<InputFlags> history;
};

struct T_InputRecorder
{
	bool is_recording = false;
	bool is_playing = false;

	RecordedInput recorded_inputs[MaxInputRecordings];
	int recording_idx = 0;
	int playing_idx = 0;
	int playing_flag_idx = 0;

public:
	static T_InputRecorder* Get()
	{
		static T_InputRecorder instance;
		return &instance;
	}
	void StartRecording();
	void Record(InputFlags flags);
	void StopRecording();
	void StartPlaying(int recording_id);
	InputFlags Play();
	void StopPlaying();
	void Save(int recording_id);
	void Load();

} inline InputRecorder;
