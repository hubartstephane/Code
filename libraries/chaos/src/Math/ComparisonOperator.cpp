#include "chaos/Chaos.h"

namespace chaos
{

	static std::vector<std::pair<ComparisonOperator, char const*>> const comparison_operator_encoding =
	{
		{ ComparisonOperator::NEVER, "NEVER" },
		{ ComparisonOperator::ALWAYS, "ALWAYS" },
		{ ComparisonOperator::EQUAL, "EQUAL" },
		{ ComparisonOperator::NOT_EQUAL, "NOT_EQUAL" },
		{ ComparisonOperator::GREATER, "GREATER" },
		{ ComparisonOperator::GREATER_EQUAL, "GREATER_EQUAL" },
		{ ComparisonOperator::LESS, "LESS" },
		{ ComparisonOperator::LESS_EQUAL, "LESS_EQUAL" }
	};

	ComparisonOperator operator ! (ComparisonOperator op)
	{
		if (op == ComparisonOperator::NEVER)
			return ComparisonOperator::ALWAYS;
		if (op == ComparisonOperator::ALWAYS)
			return ComparisonOperator::NEVER;
		if (op == ComparisonOperator::EQUAL)
			return ComparisonOperator::NOT_EQUAL;
		if (op == ComparisonOperator::NOT_EQUAL)
			return ComparisonOperator::EQUAL;
		if (op == ComparisonOperator::GREATER)
			return ComparisonOperator::LESS_EQUAL;
		if (op == ComparisonOperator::GREATER_EQUAL)
			return ComparisonOperator::LESS;
		if (op == ComparisonOperator::LESS)
			return ComparisonOperator::GREATER_EQUAL;
		if (op == ComparisonOperator::LESS_EQUAL)
			return ComparisonOperator::GREATER;
		assert(0);
		return op;
	}

	CHAOS_IMPLEMENT_ENUM_METHOD(ComparisonOperator, comparison_operator_encoding);

}; // namespace chaos
