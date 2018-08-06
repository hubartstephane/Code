#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>

#include <death/GameLevel.h>

// =================================================
// Levels
// =================================================

class LudumLevel : public death::GameLevel
{

public:

	static int const NONE = -1;
	static int const INDESTRUCTIBLE = -2;

	/** get the number of brick in the level */
	size_t GetBrickCount() const;

protected:

	/** override */
	virtual death::GameLevelInstance * DoCreateLevelInstance() override;

public:

	/** indestructible brick count */
	size_t indestructible_brick_count = 0;
	/** the brick types */
	std::vector<std::vector<int>> bricks;
};

// =================================================
// LevelInstance
// =================================================

class LudumLevelInstance : public death::GameLevelInstance
{
public:

};