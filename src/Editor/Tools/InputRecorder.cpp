#include "InputRecorder.h"

#include <sstream>
#include <iomanip>
#include <fstream>
#include "Engine/Platform/Platform.h"
#include "Engine/Rvn.h"
#include "Engine/Serialization/Parsing/Parser.h"

void RInputRecorder::StartRecording()
{
	bIsRecording = true;
}

void RInputRecorder::Record(RInputFlags Flags)
{
	RecordedInputs[RecordingIdx].History.push_back(Flags);
}

void RInputRecorder::StopRecording()
{
	bIsRecording = false;

	// trim recording (removes pauses at starting and ending)
	// we might want to move this to the play logic, so we can choose if we want to play
	// with or without empty input flags
	auto& History = RecordedInputs[RecordingIdx].History;
	// trim beginning
	{
		int I = 0;
		int P = -1;
		while (I < History.size() && History[I].KeyPress == 0)
		{
			P = I;
			I++;
		}
		if (P >= 0)
			History.erase(History.begin(), History.begin() + P);
	}
	// trim ending
	{
		int I = History.size() - 1;
		int P = -1;
		while (I >= 0 && History[I].KeyPress == 0)
		{
			P = I;
			I--;
		}
		if (P >= 0)
			History.erase(History.begin() + P, History.end());
	}

	if (History.size() > 0)
	{
		Save(RecordingIdx);
		RecordingIdx ++;
	}
}

void RInputRecorder::StartPlaying(int RecordingId)
{
	PlayingIdx = RecordingId == -1 ? RecordingIdx - 1: RecordingId;
	bIsPlaying = true;
}

RInputFlags RInputRecorder::Play()
{
	auto& Record = RecordedInputs[PlayingIdx];
	if (PlayingFlagIdx >= Record.History.size() - 1 || Record.History.size() == 0)
		StopPlaying();

	return Record.History[PlayingFlagIdx++];
}

void RInputRecorder::StopPlaying()
{
	bIsPlaying = false;
	PlayingFlagIdx = 0;
}

void RInputRecorder::Save(int RecordingId)
{
	// @std
	auto Time = std::time(nullptr);
	auto Localtime = *std::localtime(&Time);

	std::stringstream TimestampStream;
	TimestampStream << std::put_time(&Localtime, "%d-%m-%Y-%H-%M");

	std::string Timestamp = TimestampStream.str();
	std::ofstream Writer(Paths::InputRecordings + RecordingsFilenamePrefix + Timestamp + RecordingsFilenameExtension);

	if (!Writer.is_open())
		FatalError("Cant save recording to file");

	auto& Record = RecordedInputs[RecordingId].History;

	for (int i = 0; i < Record.size(); i++)
	{
		Writer << std::to_string(Record[i].KeyPress) << "\n";
		Writer << std::to_string(Record[i].KeyRelease) << "\n";
	}

}

void RInputRecorder::Load()
{
	// this will load up to memory the last MAX_INPUT_RECORDINGS recordings
	// it will *wipe* all previous in-memory stored recordings.
	// For that reason, should be used only at startup.

	vector<string> OutFiles;
	if (Platform::ListFilesInDir(Paths::InputRecordings, "*", OutFiles))
	{
		RecordingIdx = 0;

		for (auto& File : OutFiles)
		{
			Parser Parse{File};

			auto& Recording = RecordedInputs[RecordingIdx].History;
			Recording.clear();

			while (Parse.NextLine())
			{
				Parse.ParseU64();
				const uint64 KeyPress = GetParsed<uint64>(Parse);

				Parse.NextLine();
				Parse.ParseU64();
				const uint64 KeyRelease = GetParsed<uint64>(Parse);

				Recording.push_back(RInputFlags{KeyPress, KeyRelease});
			}

			RecordingIdx++;
		}
	}
}
