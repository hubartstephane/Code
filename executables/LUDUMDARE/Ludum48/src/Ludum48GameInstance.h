#pragma once

#include "Ludum48.h"
#include "Ludum48Particles.h"

// =========================================================
// LudumGameInstance
// =========================================================

class LudumGameInstance : public GameInstance
{

public:

	CHAOS_DECLARE_GAMEPLAY_FRIENDSHIPS(Ludum);

	CHAOS_DECLARE_OBJECT_CLASS2(LudumGameInstance, GameInstance);

	LudumGameInstance();

protected:

	virtual void OnLevelChanged(Level * new_level, Level * old_level, LevelInstance * new_level_instance) override;
};
