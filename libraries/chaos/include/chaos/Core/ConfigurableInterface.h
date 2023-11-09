namespace chaos
{

#ifdef CHAOS_FORWARD_DECLARATION

	class ConfigurableInterface;
	class ObjectConfigurationBase;
	class ChildObjectConfiguration;
	class RootObjectConfiguration;

	enum class InitializationType;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* InitializationType: the type of initialization
	*/

	enum class CHAOS_API InitializationType : int
	{
		INITIAL,
		HOT_RELOAD
	};

	/**
	* ConfigurableInterface: an interface representing that can have a configuration. Theses objects may react to confgiuration updates
	*/

	class CHAOS_API ConfigurableInterface
	{
		friend class ObjectConfigurationBase;

	public:

		/** destructor */
		virtual ~ConfigurableInterface() = default;

		/** getter on the configuration */
		ObjectConfigurationBase const * GetObjectConfiguration() const
		{
			return configuration.get();
		}

		ObjectConfigurationBase * GetObjectConfiguration()
		{
			return configuration.get();
		}

		/** set the configuration of the object */
		void SetObjectConfiguration(ObjectConfigurationBase* in_configuration);

		/** get the interface used for reading */
		JSONReadConfiguration GetJSONReadConfiguration() const;
		/** get the interface used for reading & writing*/
		JSONWriteConfiguration GetJSONWriteConfiguration() const;

		virtual void Initialize();

	protected:

		/** called whenever the configuration has been changed */
		virtual void OnConfigurationChanged() {}

	protected:

		/** the configuration */
		shared_ptr<ObjectConfigurationBase> configuration;
	};

	/**
	* ObjectConfigurationBase: some configuration that may be used for reading and writing (for persistent data)
	*/

	class CHAOS_API ObjectConfigurationBase : public Object
	{
		friend class ConfigurableInterface;
		friend class ChildObjectConfiguration;
		friend class RootObjectConfiguration;

	public:

		/** create a child configuration */
		ChildObjectConfiguration* CreateChildConfiguration(std::string_view key);

		/** get the interface used for reading */
		JSONReadConfiguration GetJSONReadConfiguration() const;
		/** get the interface used for reading & writing*/
		JSONWriteConfiguration GetJSONWriteConfiguration() const;

	protected:

		/** recursively send notifications */
		void PropagateNotifications();

	protected:

		/** the children configurations */
		std::vector<shared_ptr<ChildObjectConfiguration>> child_configurations;
		/** the configurable object owning this */
		weak_ptr<Object> configurable_object;

		/** the json node to read info from */
		nlohmann::json* read_config = nullptr;
		/** the json node to write info into */
		nlohmann::json* write_config = nullptr;
	};

	/**
	* ChildObjectConfiguration: a specialization that has a parent node and whose content depend on that
	*/

	class CHAOS_API ChildObjectConfiguration : public ObjectConfigurationBase
	{
		friend class ObjectConfigurationBase;
		friend class RootObjectConfiguration;

	public:

		/** destructor */
		virtual ~ChildObjectConfiguration();
		/** override */
		virtual void SubReference() override;

	protected:

		/** recursively update child configuration */
		void PropagateUpdates();
		/** update json nodes from parent configuration */
		void UpdateFromParent();
		/** remove the configuration from its parent */
		void RemoveFromParent();

	protected:

		/** the parent configuration */
		weak_ptr<ObjectConfigurationBase> parent_configuration;
		/** the path from parent to this configuration */
		std::string key;
	};

	/**
	* RootObjectConfiguration: a specialization store the whole JSON hierarchy and lend sub-content to its children
	*/

	class CHAOS_API RootObjectConfiguration : public ObjectConfigurationBase
	{
	public:

		/** constructor */
		RootObjectConfiguration();

		/** read config from files */
		bool LoadConfigurations(FilePathParam const& read_config_path, FilePathParam const& write_config_path, bool trigger_notifications = true);
		/** save the persistent data */
		bool SaveWriteConfiguration(FilePathParam const& write_config_path);
		/** call this whenever the configuration is being changed */
		void TriggerConfigurationChanged();

	protected:

		/** the json node to read info from */
		nlohmann::json root_read_config;
		/** the json node to write info into */
		nlohmann::json root_write_config;
	};

#endif

}; // namespace chaos