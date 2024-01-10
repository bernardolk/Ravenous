#include "Serialization.h"
#include <iomanip>
#include <sstream>
#include <fstream>
#include "Reflection.h"
#include "engine/rvn.h"
#include "engine/Camera/Camera.h"
#include "engine/entities/Entity.h"
#include "Engine/Platform/Platform.h"
#include "engine/world/World.h"
#include "Game/Entities/EPlayer.h"

static void LoadEditorCameraFromString(std::string& Data)
{
	std::map<string, string> Fields;
	Reflection::ParseFieldsFromSerializedObject(Data, Fields);

	auto* EditorCamera = RCameraManager::Get()->GetEditorCamera();
	
	for (auto& [Field, Value] : Fields)
	{
		if (Field == "Position")
		{
			EditorCamera->Position = Reflection::FromString<vec3>(Value);
		}
		else if (Field == "Front")
		{
			EditorCamera->Front = Reflection::FromString<vec3>(Value);			
		}
	}
}

static EEntity* LoadEntityFromString(string& Data, RWorld* World)
{
	EEntity* NewEntity = Reflection::LoadFromString(Data);
	if (!NewEntity) {
		Break("Loading entity from serialized string data failed.")
	}

	World->UpdateEntityWorldChunk(NewEntity);

	return NewEntity;
}

static string SerializeEditorCamera()
{
	string Ser;
	auto* Camera = RCameraManager::Get()->GetEditorCamera();
	Ser += "Position: vec3 = " + Reflection::ToString(Camera->Position) + "\n";
	Ser += "Front: vec3 = " + Reflection::ToString(Camera->Front) + "\n";
	return Ser;
}

void Serialization::SaveWorldToDisk()
{
	// saves each entity to its own file
	// saves camera and player data
	auto* TypeManager = Reflection::TypeMetadataManager::Get();
	auto SaveEntity = [TypeManager](EEntity* Entity)
	{
		auto* TypeMetadata = TypeManager->FindTypeMetadata(Entity->TypeID);
		if (!TypeMetadata) {
			Break("Couldn't find Type Metadata for type \"%u\"", Entity->TypeID);
			return;
		}

		string SerializedEntity = TypeMetadata->CastAndDumpFunction(*Entity);

		string QuotedUUID = Reflection::ToString(Entity->ID);
		string UnquotedUUID = QuotedUUID.substr(1, QuotedUUID.length() - 2);
		string FullPath = Paths::World + UnquotedUUID  + ".ref";
		std::ofstream Writer(FullPath);

		if (!Writer.is_open()) {
			Break("Couldn't open output filestream while trying to save entity named \"%s\"", Entity->Name.c_str());
			return;
		}

		Writer << std::fixed << std::setprecision(4);

		Writer << SerializedEntity;

		Writer.close();
	};
	
	auto EntityIterator = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		SaveEntity(Entity);
	}

	// Save player
	SaveEntity(EPlayer::Get());

	// Save camera
	{
		string FullPath = Paths::World + "Camera.rcam";
		std::ofstream Writer(FullPath);

		if (!Writer.is_open()) {
			Break("Couldn't open output filestream while trying to save the Camera");
			return;
		}

		Writer << std::fixed << std::setprecision(4);

		Writer << SerializeEditorCamera();

		Writer.close();
	}

	Log("Saved world succesfully");
}

void Serialization::LoadWorldFromDisk()
{
	auto* World = RWorld::Get();
	World->Erase();
	
	vector<string> Files;
	if (!Platform::ListFilesInDir(Paths::World, "*", Files)) {
		Break("Couldn't process files in World directory correctly.")
	}

	for (auto& File : Files)
	{
		auto Reader = std::ifstream(File);
		if (!Reader.is_open()) {
			Break("Couldn't read file \"%s\"", File.c_str())
		}

		std::stringstream ss;
		ss << Reader.rdbuf();
		Reader.close();

		string SerializedData = ss.str();
		string Extension = strrchr(&File[0], '.');
		
		if (Extension == ".ref")
		{
			auto* NewEntity = LoadEntityFromString(SerializedData, World);
			if (NewEntity->ID == 0)
			{
				EPlayer::SetPlayerSingletonInstance(NewEntity);
			}
		}

		else if (Extension == ".rcam")
		{
			LoadEditorCameraFromString(SerializedData);	
		}
	}
}

