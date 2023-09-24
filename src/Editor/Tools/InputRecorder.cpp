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

void RInputRecorder::Record(RInputFlags flags)
{
	recorded_inputs[recording_idx].history.push_back(flags);
}

void RInputRecorder::StopRecording()
{
	is_recording = false;

	// trim recording (removes pauses at starting and ending)
	// we might want to move this to the play logic, so we can choose if we want to play
	// with or without empty input flags
	auto& history = recorded_inputs[recording_idx].history;
	// trim beginning
	{
		int i = 0;
		int p = -1;
		while (i < history.size() && history[i].key_press == 0)
		{
			p = i;
			i++;
		}
		if (p >= 0)
			history.erase(history.begin(), history.begin() + p);
	}
	// trim ending
	{
		int i = history.size() - 1;
		int p = -1;
		while (i >= 0 && history[i].key_press == 0)
		{
			p = i;
			i--;
		}
		if (p >= 0)
			history.erase(history.begin() + p, history.end());
	}

	if (history.size() > 0)
	{
		Save(recording_idx);
		recording_idx ++;
	}
}

void RInputRecorder::StartPlaying(int recording_id)
{
	if (recording_id == -1)
		playing_idx = recording_idx - 1;
	else
		playing_idx = recording_id;
	is_playing = true;
}

RInputFlags RInputRecorder::Play()
{
	auto& record = recorded_inputs[playing_idx];
	if (playing_flag_idx >= record.history.size() - 1 || record.history.size() == 0)
		StopPlaying();

	return record.history[playing_flag_idx++];
}

void RInputRecorder::StopPlaying()
{
	is_playing = false;
	playing_flag_idx = 0;
}

void RInputRecorder::Save(int recording_id)
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::stringstream timestamp_stream;
	timestamp_stream << std::put_time(&tm, "%d-%m-%Y-%H-%M");

	std::string timestamp = timestamp_stream.str();
	std::ofstream writer(
		Paths::InputRecordings + RecordingsFilenamePrefix
		+ timestamp + RecordingsFilenameExtension
	);

	if (!writer.is_open())
		fatal_error("Cant save recording to file");

	auto& record = recorded_inputs[recording_id].history;

	for (int i = 0; i < record.size(); i++)
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

	vector<string> out_files;
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
