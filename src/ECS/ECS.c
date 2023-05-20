#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#define Entity unsigned int//determines max num of entities. beware. >:P

typedef struct EntityId{
	Entity id;
}EntityId;

#include "ComponentRegistry.c"
#include "Signature3.c"
#include "ECSTypes.c"
#include "ECSMacros.c"

//technically, i think the below number is a soft maximum not a soft minimum but idk
#define ECS_SOFT_MINIMUM_CHUNK_SIZE 65536 //This number I can make bigger later if I want. If I wanted to go crazy, then I could change this number per archetype, but that's a micro-opt for now. If I need that later, i'll do that later xd
#define ECS_CHUNK_PREFETCH_MULTIPLIER 2 // don't know what to make this number so I made it adjustable. Don't even know if I put this number in the right spot. TODO: Ask someone who knows more about prefetching about this number.
#define ECS_CACHE_LINE_SIZE_IN_BYTES 64 //im probs thinking way too far ahead with this one, butt oh well who cares >:P

Archetype* archetypes = NULL;
int archetypes_alloc = 0;
int archetypes_count = 0;

EntityReference* entSpots = NULL;
int entSpots_alloc = 0;
int entSpots_count = 0;

Entity* deletedEntSpots = NULL;
Entity deletedEntSpots_alloc = 0;
Entity deletedEntSpots_count = 0;

void DeleteEntitySpot(unsigned int spot){
	int delIndex = deletedEntSpots_count++;
	if(deletedEntSpots_alloc == 0){
		deletedEntSpots_alloc = 1;
		deletedEntSpots = malloc(sizeof(Entity));
	}else if(deletedEntSpots_count > deletedEntSpots_alloc - 1){
		deletedEntSpots_alloc *= 2;
		deletedEntSpots = realloc(deletedEntSpots, deletedEntSpots_alloc * sizeof(Entity));
	}
	deletedEntSpots[delIndex] = spot;
}

Entity GetOrCreateEntitySpot(){
	Entity ret = 0;
	if(deletedEntSpots_count > 0){
		ret = deletedEntSpots[--deletedEntSpots_count];
	}else{
		ret = entSpots_count++;
		if(entSpots_alloc == 0){
			entSpots_alloc = 1;
			entSpots = malloc(sizeof(EntityReference));
		}else if(entSpots_count > entSpots_alloc - 1){
			entSpots_alloc *= 2;
			entSpots = realloc(entSpots, entSpots_alloc * sizeof(EntityReference));
		}
	}
	return ret;
}



//done i think
void NewArchetype(Signature sig){
	
	//dynamic array stuff
	int oldIndex = archetypes_count++;
	if(archetypes == NULL){
		archetypes_alloc = 1;
		archetypes = malloc(sizeof(Archetype));
	}else if(archetypes_count > archetypes_alloc - 1){
		archetypes_alloc *= 2;
		archetypes = realloc(archetypes, archetypes_alloc * sizeof(Archetype));
	}
	
	//newArch->archId = oldIndex;
	
	//component offset counting
	int compCount = Sig_GetCompCount(sig);
	int* compIds = malloc(sizeof(int) * compCount);
	int* compOffsets = malloc(sizeof(int) * compCount);
	int keepTrack = 0;
	for(int i = 0; i < TOTAL_COMPONENTS; i++){
		if(Sig_TestBit(sig, i)){
			compIds[keepTrack] = i;
			keepTrack++;
		}
	}
	//calc entity size and smallest comp's size
	int entitySize = 0;
	int smallestCompSize = INT32_MAX;
	int* componentSizes = malloc(sizeof(int) * compCount);
	for(int i = 0; i < compCount; i++){
		int currCompSize = sizeOfs[compIds[i]];
		entitySize += currCompSize;
		componentSizes[i] = currCompSize;
		smallestCompSize = (smallestCompSize > currCompSize) ? currCompSize : smallestCompSize;
	}
	smallestCompSize = (smallestCompSize == 0) ? 1 : smallestCompSize;
	free(componentSizes);
	
	
	//calc the best number of entities per chunk for this archetype based off of how cpu caches work.
	int minimumEntities = ((ECS_CACHE_LINE_SIZE_IN_BYTES/smallestCompSize)+1)*ECS_CHUNK_PREFETCH_MULTIPLIER;
	int maximumEntities = ECS_SOFT_MINIMUM_CHUNK_SIZE/entitySize;
	int entsPerChunk = (maximumEntities > minimumEntities) ? maximumEntities : minimumEntities;
	
	//calculate component stream offsets in each chunk.
	int offsetTracker = 0;
	for(unsigned short i = 0; i < compCount; i++){
		compOffsets[i] = offsetTracker;
		offsetTracker += entsPerChunk * sizeOfs[compIds[i]];
	}
	
	//finalize all the new arch's data
	//I could disperse this throughout this function as-and-when these variables are calculated/made and i probably will later. But at least while i'm writing this ECS, I like having these all happen at once.
	Archetype* newArch = &archetypes[oldIndex];
	newArch->chunkAllocationSize = entsPerChunk * entitySize;
	newArch->sig = sig;
	newArch->entitiesPerChunk = entsPerChunk;
	newArch->componentCount = compCount;
	newArch->componentIds = compIds;
	newArch->componentOffsets = compOffsets;
	newArch->keepAllocatedCount = 1;
	newArch->chunks_alloc = 0;
	newArch->chunks = NULL;//calloc(newArch->chunks_alloc, sizeof(Chunk));
	newArch->chunks_count = 0;
	//newArch->chunks[0].data = malloc(newArch->chunkAllocationSize);
	
	//return newArch;
}

//I think this is done
void PopSpot(Archetype* arch){
	if(arch->chunks_count == 0){return;}
	
	Chunk* lastChunk = &arch->chunks[arch->chunks_count-1];
	lastChunk->entityCount--;
	#pragma omp critical
	{
		arch->currentEntityCount--;
	}
	
	if(lastChunk->data == NULL){ return; }//cant pop if their aint nothin to pop
	
	if(lastChunk->entityCount == 0){
		
		free(lastChunk->data);
		lastChunk->data = NULL;
		
		if(arch->chunks_count != 0){
			arch->chunks_count--;
		}
		//arch->chunks_count = (arch->chunks_count == 0) ? 0 : arch->chunks_count-1;
	}
	//print(" i mean this is workin aint it?")
}

//done i think
//TODO: turn componentIds/componentOffsets into a hash map(s).
void* GetComponentStream(int compId, t_archId archId, t_chunkId chunkId){
	Archetype* arch = &archetypes[archId];
	// if(!Sig_Equals(Sig_And(arch->sig, Sig_Create(compId)), arch->sig)){
	// 	print("lol no way this is it")
	// 	return NULL;
	// }
	if(!Sig_TestBit(arch->sig, compId)){//idk if this will speed up or slow down. probs slow down >_>
		return NULL;
	}
	for(int i = 0; i < arch->componentCount; i++){
		if(arch->componentIds[i] == compId){
			return &arch->chunks[chunkId].data[arch->componentOffsets[i]];
		}
	}
	return NULL;
}
//done?
void* GetComponent(EntityReference entRef, int compId){
	byte* stream = GetComponentStream(compId, entRef.archId, entRef.chunkId);
	if(stream == NULL){ return NULL; }
	return &stream[entRef.entId * sizeOfs[compId]];
}

//GetOrCreateArchetype(Signature sigIn)
t_archId GetOrCreateArchetype(Signature sig){
	for(t_archId i = 0; i < archetypes_count; i++){
		if(Sig_Equals(archetypes[i].sig, sig)){
			return i;//&archetypes[i];
		}
	}
	NewArchetype(sig);
	return archetypes_count-1;
}

t_chunkId NewChunk(Archetype* arch){
	t_chunkId oldIndex = arch->chunks_count++;
	if(arch->chunks == NULL){
		arch->chunks_alloc = 1;
		arch->chunks = malloc(sizeof(Chunk));
	}else if(arch->chunks_count > arch->chunks_alloc - 1){
		arch->chunks_alloc *= 2;
		arch->chunks = realloc(arch->chunks, sizeof(Chunk) * arch->chunks_alloc);
	}
	memset(&arch->chunks[oldIndex], 0, sizeof(Chunk));
	return oldIndex;
}

//GetOrCreateChunkWithSpace(Archetype* archIn)
t_chunkId GetOrCreateChunkWithSpace(Archetype* arch){
	//return 0;//todo: fix this stoupid
	
	if(arch->chunks_count == 0){
		NewChunk(arch);
		//return GetOrCreateChunkWithSpace(arch);
	}
	Chunk* lastChunk = &arch->chunks[arch->chunks_count-1];
	if(lastChunk->entityCount == arch->entitiesPerChunk){
		NewChunk(arch);
		lastChunk = &arch->chunks[arch->chunks_count-1];
	}
	if(lastChunk->data == NULL){
		lastChunk->data = malloc(arch->chunkAllocationSize);
	}
	
	return arch->chunks_count-1;//return LastChunkWithSpace
}

//done
void SetComponent(EntityReference entRef, int compId, void* compData){
	byte* stream = GetComponentStream(compId, entRef.archId, entRef.chunkId);
	if(stream == NULL){ return; }
	memcpy(&stream[entRef.entId * sizeOfs[compId]], compData, sizeOfs[compId]);
}

//I think this is done too
void copyEntity(EntityReference srcSpot, EntityReference dstSpot){
	Archetype* srcArch = &archetypes[srcSpot.archId];
	Archetype* dstArch = &archetypes[dstSpot.archId];
	for(int i = 0; i < dstArch->componentCount; i++){
		int currCompId = dstArch->componentIds[i];
		void* comp = GetComponent(srcSpot, currCompId);
		if(!comp){ continue; }
		SetComponent(dstSpot, currCompId, comp);
	}
}

void ToggleComponent(Entity ent, int compId){
	EntityReference entRefSpot = entSpots[ent];
	
	Archetype* srcArch = &archetypes[entRefSpot.archId];
	//Chunk* srcChunk = &srcArch->chunks[entRefSpot.chunkId];
	
	t_archId dstArchId = GetOrCreateArchetype(Sig_Xor(srcArch->sig, Sig_Create(compId)));
	Archetype* dstArch = &archetypes[dstArchId];
	#pragma omp critical
	{
		dstArch->currentEntityCount++;
	}
	t_chunkId dstChunkId = GetOrCreateChunkWithSpace(dstArch);
	Chunk* dstChunk = &dstArch->chunks[dstChunkId];
	
	EntityReference srcSpot = {entRefSpot.archId, entRefSpot.chunkId, entRefSpot.entId};
	EntityReference dstSpot = {.archId = dstArchId, .chunkId = dstChunkId, .entId = dstChunk->entityCount++};
	EntityReference swapSpot = {.archId = entRefSpot.archId, .chunkId = srcArch->chunks_count-1, .entId = srcArch->chunks[srcArch->chunks_count-1].entityCount-1};
	
	copyEntity(srcSpot, dstSpot);
	
	EntityId* dstIdComp = GetComponent(dstSpot, EntityId_const_enum);
	EntityReference* dstRefSpot = &entSpots[dstIdComp->id];
	memcpy(dstRefSpot, &dstSpot, sizeof(EntityReference));
	
	if(
		srcSpot.archId != swapSpot.archId ||
		srcSpot.chunkId != swapSpot.chunkId ||
		srcSpot.entId != swapSpot.entId
	){
		
		EntityId* swapIdComp = GetComponent(swapSpot, EntityId_const_enum);
		EntityReference* swapRefSpot = &entSpots[swapIdComp->id];
		memcpy(swapRefSpot, &srcSpot, sizeof(EntityReference));
		copyEntity(swapSpot, srcSpot);
	}
	
	PopSpot(&archetypes[srcSpot.archId]);
}

void Add0Component(Entity ent, int compId){
	if(Sig_TestBit(archetypes[entSpots[ent].archId].sig, compId)){ return; }
	ToggleComponent(ent, compId);
	void* compSpot = GetComponent(entSpots[ent], compId);
	memset(compSpot, 0, sizeOfs[compId]);
	// if(compData){
	// 	SetComponent(entSpots[ent], compId, compData);
	// }
	//return compSpot;
}

void AddComponent(Entity ent, int compId, void* compData){
	if(Sig_TestBit(archetypes[entSpots[ent].archId].sig, compId)){ return; }
	ToggleComponent(ent, compId);
	if(compData){
		SetComponent(entSpots[ent], compId, compData);
	}
	//return GetComponent(entSpots[ent], compId);
}

void RemoveComponent(Entity ent, int compId){
	if(!Sig_TestBit(archetypes[entSpots->archId].sig, compId)){ return; }
	ToggleComponent(ent, compId);
}

Entity CreateEntity(){
	
	t_archId zeroArchId = GetOrCreateArchetype(Sig_Create(toE(EntityId)));
	Archetype* zeroArch = &archetypes[zeroArchId];
	t_chunkId chunkId = GetOrCreateChunkWithSpace(zeroArch);
	Chunk* chunk = &zeroArch->chunks[chunkId];
	
	Entity brandNewId = GetOrCreateEntitySpot();
	entSpots[brandNewId].archId = zeroArchId;
	entSpots[brandNewId].chunkId = chunkId;
	entSpots[brandNewId].entId = chunk->entityCount++;
	
	EntityId* entId = GetComponent(entSpots[brandNewId], EntityId_const_enum);
	entId->id = brandNewId;
	
	
	return brandNewId;
	
}

void ThreadedQuery(Signature sig, void (*systemFunction)(ChunkIteratorArgs)){
	#pragma omp parallel
	{
		for(int i = 0; i < archetypes_count; i++){
			if(Sig_Match(archetypes[i].sig, sig)){
				Archetype* arch = &archetypes[i];
				#pragma omp for nowait
				for(int j = 0; j < arch->chunks_count; j++){
					Chunk* currChunk = &arch->chunks[j];
					//if(currChunk->entityCount == 0){continue;}//in theory this can go
					systemFunction((ChunkIteratorArgs){i, j});
				}
			}
		}
	}
}

void Query(Signature sig, void (*systemFunction)(ChunkIteratorArgs)){
	for(int i = 0; i < archetypes_count; i++){
		if(Sig_Match(archetypes[i].sig, sig)){
			Archetype* arch = &archetypes[i];
			for(int j = arch->chunks_count-1; j >= 0; j--){
				Chunk* currChunk = &arch->chunks[j];
				
				systemFunction((ChunkIteratorArgs){i, j});
			}
		}
	}
}



void Print_Chunk(t_archId archId, t_chunkId chunkId){
	Archetype* arch = &archetypes[archId];
	if(chunkId < 0 || chunkId >= arch->chunks_count){ print("Out of bounds chunk id %d", chunkId) }
	Chunk* chunk = &arch->chunks[chunkId];
	
	printf("Chunk:[%d]{\n",chunkId);
	printf("   ");print("dataPtr = %p;",chunk->data)
	printf("   ");print("entityCount = %d;",chunk->entityCount)
	print("}")
}

void Print_Archetype(t_archId archId){
	Archetype* arch = &archetypes[archId];
	printf("\nArchetype:[%u]:", archId);PrintSignature(arch->sig);print("{")
	printf("   ");print("currentEntityCount = %llu;",arch->currentEntityCount)
	printf("   ");print("chunks_count = %d;",arch->chunks_count)
	printf("   ");print("entitiesPerChunk = %d;",arch->entitiesPerChunk)
	printf("   ");print("componentCount = %d;",arch->componentCount)
	printf("   ");print("chunkAllocationSize in bytes = %llu;",arch->chunkAllocationSize)
	print("}")
}

void Print_ArchetypeChunks(t_archId archId){
	Archetype* arch = &archetypes[archId];
	for(int i = 0; i < arch->chunks_count; i++){
		//printf("[%d]",i);
		Print_Chunk(archId, i);
	}
	printf("\n\n\n\n");
}

void Print_EntitySpot(Entity ent){
	EntityReference entRef = entSpots[ent];
	printf("\nEntitySpot:");print("{")
	printf("   ");print("entRef.archId = %u;", entRef.archId)
	printf("   ");print("entRef.chunkId = %u;",entRef.chunkId)
	printf("   ");print("entRef.entId = %u;",entRef.entId)
	print("}")
}


// void Print_Entity(Entity entityId, char* entName){
// 	printf("\nEntity{%llu}", (size_t)entityId);
	
// }

// void Print_EntitySpot(EntitySpot entSpot, char* spotName){
// 	printf("\nEntitySpot:%s", spotName);
// 	printf(
// 		"\n{\n   archId = %d\n   chunk = %p\n   indexInChunk = %d\n}\n\n",
// 		entSpot.chunk->archId,
// 		entSpot.chunk,
// 		entSpot.indexInChunk
// 	);
// }

