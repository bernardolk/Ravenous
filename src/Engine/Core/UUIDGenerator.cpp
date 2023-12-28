#include "UUIDGenerator.h"
#include "Engine/Platform/Platform.h"
#include <random>


RUUID RUUIDGenerator::GetNewRUUID()
{
	static std::random_device RandomDevice;
	auto Seed = std::mt19937_64(RandomDevice() ^ Platform::GetCurrentSeconds() ^ Platform::GetCurrentMicroseconds());
	static std::uniform_int_distribution<uint64_t> Distr;

	return Distr(Seed);
}
