#pragma once

// ----------------
// > COLLISION LOG
// ----------------
// Holds info about the collisions that recently happened
// Basically, it contains two buffers, a main and a swap one. Once the main is filled up, it starts writing in the swap,
// while still reading from the main buffer with an offset exactly equal to the write_count in the swap buffer, like
// a moving window. Once both are completely filled, we just swap the read and write buffers so we always start reading at
// the the tail and writing at the head with the same number of total entries. 

struct CollisionLogEntry
{
	Entity* entity;
	float penetration;
	vec3 normal;
};


struct CollisionLog
{
	// buffers
	CollisionLogEntry* main;
	CollisionLogEntry* swap;
	int write_count = 0;
	// window
	CollisionLogEntry* read;
	CollisionLogEntry* write;
	const size_t window_size = COLLISION_LOG_CAPACITY;
};


// struct CollisionLogger {

//    static CollisionLog* log;



// };

//@TODO
extern CollisionLog* COLLISION_LOG;

inline void CL_log_collision(CL_Results data, int iteration)
{
	auto& log = COLLISION_LOG;
	if(log->write_count == COLLISION_LOG_CAPACITY)
	{
		// swap buffers
		if(log->read == log->write)
		{
			//log->move_window = true;
			log->write = log->swap;
		}
		else
		{
			auto temp = log->read;
			log->read = log->write;
			log->write = temp;
		}

		log->write_count = 0;
	}

	CollisionLogEntry entry;
	entry.entity = data.entity;
	entry.penetration = data.penetration;
	entry.normal = data.normal;
	log->write[log->write_count++] = entry;
}


inline CollisionLogEntry* CL_read_collision_log_entry(int i)
{
	auto& log = COLLISION_LOG;

	// out of bounds check
	if(i >= COLLISION_LOG_CAPACITY)
		return nullptr;

	// we are not using swap buffers yet
	if(log->read == log->write)
		return log->read + i;

	int read_offset = log->write_count;
	// we need to read from the read buffer
	if(read_offset + i < COLLISION_LOG_CAPACITY)
		return log->read + (read_offset + i);

	// we need to read from the write buffer
	int write_offset = COLLISION_LOG_CAPACITY - read_offset;
	return log->write + (i - write_offset);
}


inline CollisionLog* CL_allocate_collision_log()
{
	size_t size = COLLISION_LOG_CAPACITY;
	auto collision_log = new CollisionLog;
	collision_log->main = static_cast<CollisionLogEntry*>(malloc(sizeof(CollisionLogEntry) * size * 2));
	collision_log->swap = collision_log->main + size;

	// initializes memory
	for(int i = 0; i < size * 2; i++)
		collision_log->main[i] = CollisionLogEntry{NULL};

	// set ptrs
	collision_log->read = collision_log->main;
	collision_log->write = collision_log->main;

	return collision_log;
}
