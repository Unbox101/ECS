#define print(...) printf(__VA_ARGS__);printf("\n");
#include "ECS/ECS.c"
#include "Systems/ExampleSystem.c"

int main(int argc, char const *argv[])
{
	NewEntity()
	AddComp(Gravity, (Gravity){2})
	AddComp(Position, (Position){1,2,3})
	Position* posPrint = GetComp(Position)//NOTE: this pointer can and will break when we start adding and removing entities/components. But for this simple showcase, it's fine.
	
	print("pos.y = %f", posPrint->y)
	
	ExampleSystem();
	
	print("pos.y = %f", posPrint->y)
	
	return 0;
}