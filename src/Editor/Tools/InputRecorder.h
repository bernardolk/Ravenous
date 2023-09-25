#pragma once

#include "engine/core/core.h"
#include "engine/io/InputPhase.h"

constexpr static int MaxInputRecordings = 20;
const std::string RecordingsFilenamePrefix = "rec-";
const std::string RecordingsFilenameExtension = ".txt";

struct RRecordedInput
{
	std::vector<RInputFlags> history;
};

struct RInputRecorder
{
	DeclSingleton(RInputRecorder)
	bool is_recording = false;
	bool is_playing = false;

	RRecordedInput recorded_inputs[MaxInputRecordings];
	int recording_idx = 0;
	int playing_idx = 0;
	int playing_flag_idx = 0;

	void StartRecording();
	void Record(RInputFlags Flags);
	void StopRecording();
	void StartPlaying(int RecordingId);
	RInputFlags Play();
	void StopPlaying();
	void Save(int RecordingId);
	void Load();

};
