#pragma once

#include <chaos/StandardHeaders.h>

namespace chaos
{
	class KeyDefinition
	{

	public:

		/** get a key from its name */
		static int GetKey(char const* key_name);
		/** get a key name  */
		static char const * GetKeyName(int key);

	protected:

		/** the map between key and their name */
		static std::vector<std::pair<int, char const *>> const key_map;
	};

}; // namespace chaos

