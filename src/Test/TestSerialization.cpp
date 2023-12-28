#include "TestSerialization.h"

#include "engine/entities/StaticMesh.h"


static const string SerializedEntityData = 
	"Name: SolidBlock_1\n Type: EStaticMesh\n ID: \"1234-1234-1234-1234-1234\"\n Position: 42.0000 42.0000 42.0000\n Scale: 1.0000 1.0000 1.0000\n Rotation: 0.0000 0.0000 0.0000\n";

static const string SerializedPlayerData = 
	"Name: Player\n Type: EPlayer\n ID: \"0000-0000-0000-0000-0000\"\n Position: 9.9999 0.0000 0.0000\n Scale: 1.0000 1.0000 1.0000\n Rotation: 0.0000 0.0000 0.0000\n";

void RavenousTest::RunSerializationTestSuite()
{
	// 1
	Test_SerializeEntity();
}

void RavenousTest::Test_SerializeEntity()
{
	EStaticMesh TestEntity;
	TestEntity.Name = "SolidBlock_1";
	TestEntity.ID = 12341234123412341234ULL;
	TestEntity.Position = vec3{42.0f, 42.0f, 42.0f};
	TestEntity.Scale = vec3{1.0f, 1.0f, 1.0f};
	TestEntity.Rotation = vec3{0.0f, 0.0f, 0.0f};

	string SerializedEntity = Dump(TestEntity);

	auto* LoadedEntity = new EStaticMesh;
	Load(SerializedEntity, *LoadedEntity);

	for (auto& Metadata : Reflection::TypeMetadataManager::Get()->TypeMetadataCollection)
	{
		auto M = Metadata;
	}

	int Noop = 42;
}