namespace chaos
{

#ifdef CHAOS_FORWARD_DECLARATION

	enum class ReadConfigurablePropertiesContext;

	class ConfigurableInterface;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	*  ReadConfigurablePropertiesContext: the situation when readding properties is done
	*/

	enum class ReadConfigurablePropertiesContext : int
	{
		INITIALIZATION,
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

		/** reload the configuration for this object and its children */
		bool ReloadObjectConfiguration(bool send_notifications);

		/** create a child configuration and give it to another object */
		bool GiveChildConfiguration(ConfigurableInterface* other_configurable, std::string key);
		/** read the properties from the config */
		bool ReadConfigurableProperties(ReadConfigurablePropertiesContext context = ReadConfigurablePropertiesContext::INITIALIZATION);
		/** store the properties into the persistent data */
		bool StorePersistentProperties();

	protected:

		/** called whenever the configuration has been changed */
		virtual bool OnConfigurationChanged(JSONReadConfiguration read_config);
		/** the reading properties method */
		virtual bool OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context);
		/** prepare persistent properties for saving */
		virtual bool OnStorePersistentProperties(JSONWriteConfiguration config);

	protected:

		/** the configuration */
		shared_ptr<ObjectConfigurationBase> configuration;
	};

#endif

}; // namespace chaos