

#define toE(typeName) typeName##_enum
#define streamOf(typeName) STREAM_##typeName##_enum

//  query/system related macros
#define floop Archetype* _arch = &archetypes[iterArgs.arch]; \
Chunk* _chunk = &_arch->chunks[iterArgs.chunk]; \
for(int entIndex = _chunk->entityCount-1; entIndex >= 0; entIndex--)
	
#define GetStream(x) byte* streamOf(x) = GetComponentStream(toE(x), iterArgs.arch, iterArgs.chunk);
#define UseStream(x) (void*)&streamOf(x)[entIndex * sizeOfs[toE(x)] ];
//#define LazyUseStreamType(typeName, varName, comp) typeName* varName = (typeName*)UseStream(comp);

//  component/entity related macros
#define b_NewEntity CreateEntity()
#define b_AddComp(entName, compTypeName, ...) AddComponent(entName, toE(compTypeName), &__VA_ARGS__)
#define b_AddComp0(entName, compTypeName) Add0Component(entName, toE(compTypeName))
#define b_GetComp(entity, compTypeName) GetComponent(entSpots[entity], toE(compTypeName))

//CUSTOM bindful aliases of the above macros (Change or add to these to your liking.)
Entity _boundEntity;
#define NewEntity() _boundEntity = b_NewEntity;
#define AddComp(compTypeName, ...) b_AddComp(_boundEntity, compTypeName, __VA_ARGS__);
#define AddComp0(compTypeName) b_AddComp0(_boundEntity, compTypeName);
#define GetComp(compTypeName) b_GetComp(_boundEntity, compTypeName);


//Preprocessor pragmas
#define macro_A1(comp) comp _##comp;
#define macro_A2(comp, varName) comp varName;
#define macro_A3(comp, varName, newType) newType varName;

#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define macro_AChooser(...) \
	GET_4TH_ARG(__VA_ARGS__, macro_A3, macro_A2, macro_A1)


#define needsComp(...) 
#define usingComp(...) macro_AChooser(__VA_ARGS__)(__VA_ARGS__)

//aliases cuz im lazy
#define NeedsComp needsComp
#define UsingComp usingComp



#define CreateAndBindEntity() _boundEntity = CreateEntity();
#define BindEntity(ent) _boundEntity = ent;



#define CHUNKSIZE 65536
#define EntityId_const_enum EntityId_enum

