#include "Ludum45PCH.h"
#include "Ludum45Game.h"

enum class Truc : int
{
	None = 0,
	Truc1 = 1,
	Truc2 = 2,
	Truc3 = 4,
	Truc4 = 8
};


static chaos::EnumTools::EnumMetaData<Truc> const Truc_metadata =
{
	{ Truc::Truc1, "Truc1" },
	{ Truc::Truc2, "Truc2" },
	{ Truc::Truc3, "Truc3" },
	{ Truc::Truc4, "Truc4" }
};

CHAOS_DECLARE_ENUM_METHOD(Truc);
CHAOS_IMPLEMENT_ENUM_METHOD(Truc, &Truc_metadata);

CHAOS_GLOBAL_VARIABLE(Truc, mtruc);
CHAOS_GLOBAL_VARIABLE(bool, mybool);
CHAOS_GLOBAL_VARIABLE(int, myint, 666);
CHAOS_GLOBAL_VARIABLE(Truc, mytruc2, Truc::Truc3);
CHAOS_GLOBAL_VARIABLE(chaos::Hotpoint, hot);


int main(int argc, char ** argv, char ** env)
{
	return chaos::RunGame<LudumGame>(argc, argv, env);
}
