#include "InputRecorder.h"

#include <sstream>
#include <iomanip>

#include "Engine/Platform/Platform.h"
#include "Engine/Rvn.h"
#include "Engine/Serialization/Parsing/Parser.h"

RInputRecorder::RInputRecorder() = default;

void RInputRecorder::StartRecording()
{
	is_recording = true;
}

void RInputRecorder::Record(RInputFlags Flags)
{
	recorded_inputs[recording_idx].history.push_back(flags);
}

void RInputRecorder::StopRecording()
{
	is_recording = false;

	// trim recording (removes pauses at starting and ending)
	// we might want to move this to the play logic, so we can choose if we want to play
	// with or without empty input flags
	auto& History = recorded_inputs[recording_idx].history;
	// trim beginning
	{
		int I = 0;
		int P = -1;
		while (I < history.size() && history[I].key_press == 0)
		{
			P = I;
			I++;
		}
		if (P >= 0)
			history.erase(history.begin(), history.begin() + P);
	}
	// trim ending
	{
		int I = history.size() - 1;
		int P = -1;
		while (I >= 0 && history[I].key_press == 0)
		{
			P = I;
			I--;
		}
		if (P >= 0)
			history.erase(history.begin() + P, history.end());
	}

	if (history.size() > 0)
	{
		Save(recording_idx);
		recording_idx ++;
	}
}

void RInputRecorder::StartPlaying(int RecordingId)
{
	if (RecordingId == -1)
		playing_idx = recording_idx - 1;
	else
		playing_idx = RecordingId;
	is_playing = true;
}

RInputFlags RInputRecorder::Play()
{
	auto& Record = recorded_inputs[playing_idx];
	if (playing_flag_idx >= record.history.size() - 1 || record.history.size() == 0)
		StopPlaying();

	return record.history[playing_flag_idx++];
}

void RInputRecorder::StopPlaying()
{
	is_playing = false;
	playing_flag_idx = 0;
}

void RInputRecorder::Save(int RecordingId)
{
	auto T = std::time(nullptr);
	auto Tm = *std::localtime(&t);

	std::stringstream TimestampStream;
	timestamp_stream << std::put_time(&tm, "%d-%m-%Y-%H-%M");

	std::string Timestamp = timestamp_stream.str();
	std::ofstream writer(
		Paths::InputRecordings + RecordingsFilenamePrefix
		+ timestamp + RecordingsFilenameExtension
	);

	if (!writer.is_open())
		fatal_error("Cant save recording to file");

	auto& Record = recorded_inputs[recording_id].history;

	for (int I = 0; I < record.size(); I++)
	{
		writer << std::to_string(record[i].key_press) << "\n";
		writer << std::to_string(record[i].key_release) << "\n";
	}

}

void RInputRecorder::Load()
{
	// this will load up to memory the last MAX_INPUT_RECORDINGS recordings
	// it will *wipe* all previous in-memory stored recordings.
	// For that reason, should be used only at startup.

	vector<string> OutFiles;
	if (Platform::ListFilesInDir(Paths::InputRecordings, "*", out_files))
	{
		recording_idx = 0;

		for (auto& file : out_files)
		{
			Parser p{file};

			auto& recording = recorded_inputs[recording_idx].history;
			recording.clear();

			while (p.NextLine())
			{
				p.ParseU64();
				const uint64 key_press = GetParsed<uint64>(p);

				p.NextLine();
				p.ParseU64();
				const uint64 key_release = GetParsed<uint64>(p);

				recording.push_back(RInputFlags{key_press, key_release});
			}

			recording_idx++;
		}
	}
}
