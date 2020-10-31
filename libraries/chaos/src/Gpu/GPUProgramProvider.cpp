﻿#include <chaos/Chaos.h>

namespace chaos
{

	bool GPUProgramProviderTexture::DoProcessAction(char const * name, GPUProgramAction & action, GPUProgramProviderBase const * top_provider) const
	{
		if (name != nullptr && handled_name != name)
			return false;
		return action.Process(name, value.get(), this);
	}

	void GPUProgramProvider::Clear()
	{
		children_providers.clear();
	}

	void GPUProgramProvider::AddVariableProvider(GPUProgramProviderBase * provider)
	{
		if (provider != nullptr)
			children_providers.push_back(provider);
	}

	bool GPUProgramProvider::DoProcessAction(char const * name, GPUProgramAction & action, GPUProgramProviderBase const * top_provider) const
	{
		// handle children providers
		size_t count = children_providers.size();
		for (size_t i = 0; i < count; ++i)
			if (children_providers[i].get()->DoProcessAction(name, action, top_provider))
				return true;
		// failure
		return false;
	}

	bool GPUProgramProviderChain::DoProcessAction(char const * name, GPUProgramAction & action, GPUProgramProviderBase const * top_provider) const
	{
		// use variables inside this provider
		if (GPUProgramProvider::DoProcessAction(name, action, top_provider))
			return true;
		// use fallback provider
		if (fallback_provider != nullptr)
			if (fallback_provider->DoProcessAction(name, action, top_provider))
				return true;
		return false;
	}


}; // namespace chaos