#include "chaos/ChaosPCH.h"
#pragma once

#include <chaos/chaos.h>

namespace chaos
{
	// ---------------------------------------------------------------------
	// ConfigurableInterface implementation
	// ---------------------------------------------------------------------

	void ConfigurableInterface::SetObjectConfiguration(ObjectConfigurationBase* in_configuration)
	{
		if (configuration.get() != in_configuration)
		{
			// erase previous configuration
			if (configuration != nullptr)
				configuration->configurable_object = nullptr;
			// set new configuration
			configuration = in_configuration;
			if (configuration != nullptr)
				configuration->configurable_object = auto_cast(this);
		}
	}

	JSONReadConfiguration ConfigurableInterface::GetJSONReadConfiguration() const
	{
		if (configuration != nullptr)
			return configuration->GetJSONReadConfiguration();
		return {};
	}

	JSONWriteConfiguration ConfigurableInterface::GetJSONWriteConfiguration() const
	{
		if (configuration != nullptr)
			return configuration->GetJSONWriteConfiguration();
		return {};
	}

	void ConfigurableInterface::OnConfigurationChanged()
	{
		InitializeFromConfiguration(, InitializationType::HOT_RELOAD)
	
	}

	// ---------------------------------------------------------------------
	// ObjectConfigurationBase implementation
	// ---------------------------------------------------------------------

	ChildObjectConfiguration* ObjectConfigurationBase::CreateChildConfiguration(std::string_view key)
	{
		if (ChildObjectConfiguration* result = new ChildObjectConfiguration)
		{
			child_configurations.push_back(result);

			result->parent_configuration = this;
			result->key = key;
			result->UpdateFromParent();

			return result;
		}
		return nullptr;
	}

	void ObjectConfigurationBase::PropagateNotifications()
	{
		// trigger the change for the configurable
		if (ConfigurableInterface* configurable = auto_cast(configurable_object.get()))
			configurable->OnConfigurationChanged();
		// create a weak copy of the children list. children may be destroyed during this loop
		std::vector<weak_ptr<ChildObjectConfiguration>> child_copy;
		child_copy.reserve(child_configurations.size());
		for (auto& child : child_configurations)
			child_copy.push_back(child.get());
		// propagate the change to sub hierarchy
		for (auto& child : child_copy)
			if (child != nullptr)
				child->PropagateNotifications();
	}

	JSONReadConfiguration ObjectConfigurationBase::GetJSONReadConfiguration() const
	{
		JSONReadConfiguration result;
		result.read_config = read_config;
		result.write_config = write_config;
		return result;
	}

	JSONWriteConfiguration ObjectConfigurationBase::GetJSONWriteConfiguration() const
	{
		JSONWriteConfiguration result;
		result.read_config = read_config;
		result.write_config = write_config;
		return result;
	}

	// ---------------------------------------------------------------------
	// ChildObjectConfiguration implementation
	// ---------------------------------------------------------------------

	ChildObjectConfiguration::~ChildObjectConfiguration()
	{
		assert(parent_configuration == nullptr);
	}

	void ChildObjectConfiguration::SubReference()
	{
		if (parent_configuration == nullptr)
			Object::SubReference(); // the configuration is handled as usual
		else if (--shared_count == 1) // the last reference is the one from the parent. Destroy it 
			RemoveFromParent();
	}

	void ChildObjectConfiguration::RemoveFromParent()
	{
		assert(parent_configuration != nullptr);

		auto it = std::ranges::find_if(parent_configuration->child_configurations, [this](auto const& p) { return p == this; }); // search this in our parent's children list
		if (it != parent_configuration->child_configurations.end())
			parent_configuration->child_configurations.erase(it);

		parent_configuration = nullptr;
	}

	void ChildObjectConfiguration::UpdateFromParent()
	{
		read_config = (parent_configuration != nullptr && parent_configuration->read_config != nullptr) ?
			JSONTools::GetStructure(*parent_configuration->read_config, key) :
			nullptr;
		write_config = (parent_configuration != nullptr && parent_configuration->write_config != nullptr) ?
			JSONTools::GetOrCreateStructure(*parent_configuration->write_config, key) :
			nullptr;
	}

	void ChildObjectConfiguration::PropagateUpdates()
	{
		// update the configuration
		UpdateFromParent();
		// propagate the update to sub hierarchy
		for (shared_ptr<ChildObjectConfiguration>& child : child_configurations)
			child->PropagateUpdates();
	}

	// ---------------------------------------------------------------------
	// RootObjectConfiguration implementation
	// ---------------------------------------------------------------------

	RootObjectConfiguration::RootObjectConfiguration()
	{
		read_config = &root_read_config;
		write_config = &root_write_config;
	}

	void RootObjectConfiguration::TriggerConfigurationChanged()
	{
		// propagate the update to sub hierarchy
		for (shared_ptr<ChildObjectConfiguration>& child : child_configurations)
			child->PropagateUpdates();
		// send notifications
		PropagateNotifications();
	}

	bool RootObjectConfiguration::LoadConfigurations(FilePathParam const& read_config_path, FilePathParam const& write_config_path, bool trigger_notifications)
	{
		// update the read json
		root_read_config = nlohmann::json();
		JSONTools::LoadJSONFile(read_config_path, root_read_config, LoadFileFlag::RECURSIVE);

		// update the write json
		root_write_config = nlohmann::json();
		JSONTools::LoadJSONFile(write_config_path, root_write_config, LoadFileFlag::RECURSIVE);

		// notify the changes
		if (trigger_notifications)
			TriggerConfigurationChanged();

		return true;
	}

	bool RootObjectConfiguration::SaveWriteConfiguration(FilePathParam const& write_config_path)
	{
		return JSONTools::SaveJSONToFile(root_write_config, write_config_path);
	}

}; // namespace chaos