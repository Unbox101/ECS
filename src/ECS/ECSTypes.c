#pragma once

#define t_archId unsigned int
#define t_chunkId unsigned int
#define t_entId unsigned int

#ifndef byte
#define byte uint8_t
#endif

typedef struct Chunk{
	unsigned short entityCount;
	byte* data;
}Chunk;

typedef struct EntityReference{
	t_archId archId;
	t_chunkId chunkId;
	t_entId entId;
}EntityReference;

typedef struct Archetype{
	//int archId;
	Signature sig;//TODO: Break this out into a seperate dynamic array for a prefetching bonus on the linear iteration of signatures required for Querys.
	//To explain the above a bit more, This is an optimization for the linear search of archetypes/signatures we do per query, in an effort to not need to implement a hashmap right away. I could implement one later and it *might maybe* speed this query/search part up as the archetype count grows, but how many archetypes are we actually gonna have? Like, this is just not the biggest bottleneck, so I am not going to focus on it right now.
	
	
	
	int entitiesPerChunk;
	size_t chunkAllocationSize;
	
	int componentCount;
	int* componentIds;
	int* componentOffsets;
	
	
	int keepAllocatedCount;
	Chunk* chunks;
	int chunks_alloc;
	int chunks_count;
	
	
	unsigned long long currentEntityCount;
	//size_t* compOffsetMap;
}Archetype;

typedef struct{
	t_archId arch;
	t_chunkId chunk;
}ChunkIteratorArgs;


//TODO:
// typedef struct ECSCommand{
// 	Entity entity;
// 	int compType;
// }ECSCommand;

//TODO:
// typedef struct EntityCommandQueue{
// 	ECSCommand* queue;
// 	int queue_alloc;
// 	int queue_count;
// }EntityCommandQueue;