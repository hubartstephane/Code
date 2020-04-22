#pragma once

#include <chaos/StandardHeaders.h>

namespace chaos
{
	enum class WrapMode : int
	{
		none = 0,
		clamp = 1,
		wrap = 2,
		check_ranges = 3
	};

	// apply the wrap mode (return whether this is not an out of bounds)
	template<typename T>
	bool ApplyWrapMode(T src, T min_value, T max_value, WrapMode mode, T & result)
	{
		if (mode == WrapMode::none)
		{
			result = src;
		}
		else if (mode == WrapMode::clamp)
		{
			result = std::clamp(src, min_value, max_value);
		}
		else if (mode == WrapMode::wrap)
		{
			result = min_value + MathTools::Modulo(src - min_value, max_value - min_value + 1); // +1 because 'max_value' is inside accepted range
		}
		else if (mode == WrapMode::check_ranges)
		{
			if (src < min_value)
				return false;
			if (src > max_value)
				return false;
			result = src;
		}
		return true; // except for ranges, this function should always be successful 
	}

	template<typename T>
	bool ApplyWrapMode(glm::tvec1<T> const & src, glm::tvec1<T> const & min_value, glm::tvec1<T> const& max_value, WrapMode mode, glm::tvec1<T> & result)
	{
		for (size_t i = 0; i < src.length(); ++i)
			if (!ApplyWrapMode(src[i], min_value[i], max_value[i], mode, result[i]))
				return false;
		return true;
	}

	template<typename T>
	bool ApplyWrapMode(glm::tvec2<T> const & src, glm::tvec2<T> const& min_value, glm::tvec2<T> const& max_value, WrapMode mode, glm::tvec2<T>& result)
	{
		for (size_t i = 0; i < src.length(); ++i)
			if (!ApplyWrapMode(src[i], min_value[i], max_value[i], mode, result[i]))
				return false;
		return true;
	}

	template<typename T>
	bool ApplyWrapMode(glm::tvec3<T> const & src, glm::tvec3<T> const& min_value, glm::tvec3<T> const& max_value, WrapMode mode, glm::tvec3<T>& result)
	{
		for (size_t i = 0; i < src.length(); ++i)
			if (!ApplyWrapMode(src[i], min_value[i], max_value[i], mode, result[i]))
				return false;
		return true;
	}

	template<typename T>
	bool ApplyWrapMode(glm::tvec4<T> src, glm::tvec4<T> const& min_value, glm::tvec4<T> const& max_value, WrapMode mode, glm::tvec4<T>& result)
	{
		for (size_t i = 0; i < src.length(); ++i)
			if (!ApplyWrapMode(src[i], min_value[i], max_value[i], mode, result[i]))
				return false;
		return true;
	}

	/** JSON loading method */
	bool LoadFromJSON(nlohmann::json const& json_entry, WrapMode& dst);
	/** JSON saving method */
	bool SaveIntoJSON(nlohmann::json& json_entry, WrapMode const& src);

}; // namespace chaos
