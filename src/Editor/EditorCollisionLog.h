#pragma once

// ----------------
// > COLLISION LOG
// ----------------
// Holds info about the collisions that recently happened
// Basically, it contains two buffers, a main and a swap one. Once the main is filled up, it starts writing in the swap,
// while still reading from the main buffer with an offset exactly equal to the write_count in the swap buffer, like
// a moving window. Once both are completely filled, we just swap the read and write buffers so we always start reading at
// the the tail and writing at the head with the same number of total entries.

struct RCollisionLogEntry
{
	EEntity* Entity;
	float Penetration;
	vec3 Normal;
};


struct RCollisionLog
{
	// buffers
	RCollisionLogEntry* Main;
	RCollisionLogEntry* Swap;
	int WriteCount = 0;
	// window
	RCollisionLogEntry* Read;
	RCollisionLogEntry* Write;
	const u32 WindowSize = COLLISION_LOG_CAPACITY;
};


// struct CollisionLogger {

//    static CollisionLog* log;



// };

//@TODO
extern RCollisionLog* CollisionLog;

inline void ClLogCollision(CL_Results Data, int Iteration)
{
	auto& Log = CollisionLog;
	if (Log->WriteCount == COLLISION_LOG_CAPACITY)
	{
		// swap buffers
		if (Log->Read == Log->Write)
		{
			//log->move_window = true;
			Log->Write = Log->Swap;
		}
		else
		{
			auto Temp = Log->Read;
			Log->Read = Log->Write;
			Log->Write = Temp;
		}

		Log->WriteCount = 0;
	}

	RCollisionLogEntry Entry;
	Entry.Entity = Data.entity;
	Entry.Penetration = Data.penetration;
	Entry.Normal = Data.normal;
	Log->Write[Log->WriteCount++] = Entry;
}


inline RCollisionLogEntry* ClReadCollisionLogEntry(int Index)
{
	auto& Log = CollisionLog;

	// out of bounds check
	if (Index >= COLLISION_LOG_CAPACITY)
		return nullptr;

	// we are not using swap buffers yet
	if (Log->Read == Log->Write)
		return Log->Read + Index;

	int ReadOffset = Log->WriteCount;
	// we need to read from the read buffer
	if (ReadOffset + Index < COLLISION_LOG_CAPACITY)
		return Log->Read + (ReadOffset + Index);

	// we need to read from the write buffer
	int WriteOffset = COLLISION_LOG_CAPACITY - ReadOffset;
	return Log->Write + (Index - WriteOffset);
}


inline RCollisionLog* ClAllocateCollisionLog()
{
	u32 Size = COLLISION_LOG_CAPACITY;
	auto CollisionLog = new RCollisionLog;
	CollisionLog->Main = static_cast<RCollisionLogEntry*>(malloc(sizeof(RCollisionLogEntry) * Size * 2));
	CollisionLog->Swap = CollisionLog->Main + Size;

	// initializes memory
	for (int i = 0; i < Size * 2; i++)
		collision_log->main[i] = RCollisionLogEntry{NULL};

	// set ptrs
	CollisionLog->Read = CollisionLog->Main;
	CollisionLog->Write = CollisionLog->Main;

	return CollisionLog;
}
