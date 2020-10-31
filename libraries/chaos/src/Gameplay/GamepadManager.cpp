#include <chaos/Chaos.h>

namespace chaos
{
	bool GameGamepadManager::DoPoolGamepad(MyGLFW::PhysicalGamepad * physical_gamepad)
	{
		if (game != nullptr)
			return game->OnPhysicalGamepadInput(physical_gamepad);
		return true;
	}

}; // namespace chaos
