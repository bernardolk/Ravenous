#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>
#include "engine/serialization/parsing/parser.h"

constexpr static int MaxInputRecordings = 20;
const std::string RecordingsFilenamePrefix = "rec-";
const std::string RecordingsFilenameExtension = ".txt";

struct RecordedInput
{
	std::vector<InputFlags> history;
};

// ReSharper disable once CppInconsistentNaming
struct T_InputRecorder
{
	bool is_recording = false;
	bool is_playing = false;

	RecordedInput recorded_inputs[MaxInputRecordings];
	int recording_idx = 0;
	int playing_idx = 0;
	int playing_flag_idx = 0;

	void StartRecording()
	{
		is_recording = true;
	}

	void Record(InputFlags flags)
	{
		recorded_inputs[recording_idx].history.push_back(flags);
	}

	void StopRecording()
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
			while(i < history.size() && history[i].key_press == 0)
			{
				p = i;
				i++;
			}
			if(p >= 0)
				history.erase(history.begin(), history.begin() + p);
		}
		// trim ending
		{
			int i = history.size() - 1;
			int p = -1;
			while(i >= 0 && history[i].key_press == 0)
			{
				p = i;
				i--;
			}
			if(p >= 0)
				history.erase(history.begin() + p, history.end());
		}

		if(history.size() > 0)
		{
			Save(recording_idx);
			recording_idx ++;
		}
	}

	void StartPlaying(int recording_id)
	{
		if(recording_id == -1)
			playing_idx = recording_idx - 1;
		else
			playing_idx = recording_id;
		is_playing = true;
	}

	InputFlags Play()
	{
		auto& record = recorded_inputs[playing_idx];
		if(playing_flag_idx >= record.history.size() - 1 || record.history.size() == 0)
			StopPlaying();

		return record.history[playing_flag_idx++];
	}

	void StopPlaying()
	{
		is_playing = false;
		playing_flag_idx = 0;
	}

	void Save(int recording_id)
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

		if(!writer.is_open())
		{
			std::cout << "Cant save recording to file. \n";
			assert(false);
		}

		auto& record = recorded_inputs[recording_id].history;

		for(int i = 0; i < record.size(); i++)
		{
			writer << std::to_string(record[i].key_press) << "\n";
			writer << std::to_string(record[i].key_release) << "\n";
		}

	}

	void Load()
	{
		// this will load up to memory the last MAX_INPUT_RECORDINGS recordings
		// it will *wipe* all previous in-memory stored recordings.
		// For that reason, should be used only at startup.

		std::vector<std::string> files;
		if(OS_list_files(Paths::InputRecordings, "*", files))
		{
			recording_idx = 0;

			for(auto& file : files)
			{
				Parser p{file};

				auto& recording = recorded_inputs[recording_idx].history;
				recording.clear();

				while(p.NextLine())
				{
					p.ParseU64();
					const u64 key_press = get_parsed<u64>(p);

					p.NextLine();
					p.ParseU64();
					const u64 key_release = get_parsed<u64>(p);

					recording.push_back(InputFlags{key_press, key_release});
				}

				recording_idx++;
			}
		}
	}


	// void _get_file_id_from_filename(string filename)
	// {
	//    // get the last record id saved
	//    std::vector<std::string> recordings;
	//    if (OS_list_files(Paths::InputRecordings, "*", files)) 
	//    {
	//       for (int i = 0; i < recordings.size(); i++)
	//       {
	//          size_t pos = recordings[i].find(RECORDINGS_FILENAME_PREFIX);
	//         std::stringfilename = recordings[i].substr(pos);
	//          size_t
	//    }
	// }

} inline InputRecorder;
