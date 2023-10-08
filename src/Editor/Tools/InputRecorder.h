#pragma once

#include "engine/core/core.h"
#include "engine/io/InputPhase.h"

constexpr static int MaxInputRecordings = 20;
const string RecordingsFilenamePrefix = "rec-";
const string RecordingsFilenameExtension = ".txt";

struct RRecordedInput
{
	vector<RInputFlags> History;
};

struct RInputRecorder
{
	static RInputRecorder* Get()
	{
		static RInputRecorder Instance{};
		return &Instance;
	}
	
	bool bIsRecording = false;
	bool bIsPlaying = false;

	RRecordedInput RecordedInputs[MaxInputRecordings];
	int RecordingIdx = 0;
	int PlayingIdx = 0;
	int PlayingFlagIdx = 0;

	void StartRecording();
	void Record(RInputFlags Flags);
	void StopRecording();
	void StartPlaying(int RecordingId);
	RInputFlags Play();
	void StopPlaying();
	void Save(int RecordingId);
	void Load();

};
