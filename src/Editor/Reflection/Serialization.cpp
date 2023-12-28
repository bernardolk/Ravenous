#include "Serialization.h"
#include <iomanip>
#include <sstream>

#include "Reflection.h"
#include "engine/rvn.h"
#include "engine/entities/Entity.h"
#include "Engine/Platform/Platform.h"
#include "engine/world/World.h"


void Serialization::SaveWorldToDisk()
{
	auto* TypeManager = Reflection::TypeMetadataManager::Get();
	auto EntityIterator = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		auto* TypeMetadata = TypeManager->FindTypeMetadata(Entity->TypeID);
		if (!TypeMetadata) {
			Break("Couldn't find Type Metadata for type \"%u\"", Entity->TypeID);
			continue;
		}

		string SerializedEntity = TypeMetadata->CastAndDumpFunction(*Entity);

		string QuotedUUID = Reflection::ToString(Entity->ID);
		string UnquotedUUID = QuotedUUID.substr(1, QuotedUUID.length() - 2);
		string FullPath = Paths::World + UnquotedUUID  + ".ref";
		std::ofstream Writer(FullPath);

		if (!Writer.is_open()) {
			Break("Couldn't open output filestream while trying to save entity named \"%s\"", Entity->Name.c_str());
			continue;
		}

		Writer << std::fixed << std::setprecision(4);

		Writer << SerializedEntity;

		Writer.close();
	}
}


void Serialization::LoadWorldFromDisk()
{	
	vector<string> Files;
	if (!Platform::ListFilesInDir(Paths::World, "*", Files)) {
		Break("Couldn't process files in World directory correctly.")
	}

	auto* World = RWorld::Get();
	for (auto& File : Files)
	{
		auto Reader = std::ifstream(File);

		if (!Reader.is_open()) {
			Break("Couldn't read file \"%s\"", File.c_str())
		}

		std::stringstream ss;
		ss << Reader.rdbuf();
		Reader.close();

		string SerializedEntity = ss.str();

		EEntity* NewEntity = Reflection::LoadFromString(SerializedEntity);
		if (!NewEntity) {
			Break("Loading entity from serialized string data failed.")
		}

		World->UpdateEntityWorldChunk(NewEntity);
	}
}