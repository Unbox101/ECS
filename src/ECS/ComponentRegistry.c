//This file normally gets generated automatically based on everywhere we type "#pragma comp(struct_name)" or a short hand version "#pragma comp" that implicitely uses the struct directly below it.
#include "../Components/Gravity.c"
#include "../Components/Position.c"
#include "../Components/ExampleTag.c"

#define EntityId_enum 0
#define Gravity_enum 1
#define Position_enum 2
#define ExampleTag_enum 3
#define TOTAL_COMPONENTS 4

const unsigned int sizeOfs[] = {
	#ifndef EntityId
		sizeof(EntityId),
	#else
		0,//This is for if the component is a tag.
	#endif
	#ifndef Gravity
		sizeof(Gravity),
	#else
		0,
	#endif
	#ifndef Position
		sizeof(Position),
	#else
		0,
	#endif
	#ifndef ExampleTag
		sizeof(ExampleTag),
	#else
		0,
	#endif
};
