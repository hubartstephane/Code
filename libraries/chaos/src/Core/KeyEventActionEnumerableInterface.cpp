#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	bool KeyEventActionProcessor::operator() (KeyboardButton button, int modifier, char const* name, std::function<void()> const& func)
	{
		return false;
	}

	bool KeyEventActionEnumerableInterface::EnumerateKeyEventActions(KeyEventActionProcessor & processor)
	{
		return false;
	}

}; // namespace chaos
