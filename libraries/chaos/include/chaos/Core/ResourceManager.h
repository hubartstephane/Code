namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class ResourceManager;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	// ==============================================================
	// MANAGER
	// ==============================================================

	class CHAOS_API ResourceManager : public Object
	{

	public:

		/** start the manager */
		bool StartManager();
		/** stop the manager */
		bool StopManager();

		/** whether the manager is started */
		bool IsManagerStarted() const;

		/** initialize the manager from a configuration file */
		virtual bool InitializeFromConfiguration();

	protected:

		/** internally starts the manager */
		virtual bool DoStartManager();
		/** internally stops the manager */
		virtual bool DoStopManager();

		/** load all files from a directory */
		template<typename LOADER>
		static void LoadObjectsInDirectory(FilePathParam const & path, LOADER const& loader, bool recurse = false)
		{
			FileTools::WithDirectoryContent(path, [&loader, recurse](boost::filesystem::path const& p)
			{
				// copy loader to ensure content is reset for all loaded objects
				LOADER other_loader = loader;
				if (other_loader.LoadObject(p) == nullptr && recurse) // let loader decides whether it accepts a directory as a valid path
				{
					// if it is a directory and loader fails, recurse inside it
					boost::filesystem::file_status status = boost::filesystem::status(p);
					if (status.type() == boost::filesystem::file_type::directory_file)
						LoadObjectsInDirectory(p, loader);
				}
				return false; // don't stop
			});
		}

		/** an utility method to load a whole directory from a json entry */
		template<typename LOADER>
		static void DoLoadObjectsRecurseDirectories(nlohmann::json const& json, LOADER const& loader)
		{
			std::vector<std::string> paths;
			if (LoadFromJSON(json, paths))
				for (std::string const& directory_name : paths)
					LoadObjectsInDirectory(directory_name, loader, true);
		}

		/** an utility method to initialize a single object in an JSON array/object */
		template<bool RECURSE, typename LOADER>
		static auto DoLoadObjectsFromConfiguration(char const* name, nlohmann::json const& json, LOADER const & loader) -> typename LOADER::resource_type* // LOADER passed by copy is important to ensure reset for all loaded objects
		{
			// 1 - recurse over some directories
			if constexpr (RECURSE)
			{
				if (name != nullptr && StringTools::Stricmp(name, "[recurse]") == 0)
				{
					DoLoadObjectsRecurseDirectories(json, loader);
					return nullptr;
				}
			}
			// 2 - we receive a key and its is valid (starts with '@')
			if (name != nullptr && name[0] == '@' && name[1] != 0)
			{
				// copy loader to ensure content is reset for all loaded objects
				LOADER other_loader = loader;
				return other_loader.LoadObject(name + 1, json);
			}
			// 3 - try to find a member 'name'
			std::string resource_name;
			if (JSONTools::GetAttribute(json, "name", resource_name))
			{
				if (!resource_name.empty())
					return loader.LoadObject(resource_name.c_str(), json);
			}
			// 4 - anonymous object
			return loader.LoadObject(nullptr, json);
		}

		/** an utility method to initialize a list of objects from a JSON object or array */
		template<bool RECURSE, typename LOADER>
		static bool LoadObjectsFromConfiguration(char const* object_names, JSONReadConfiguration config, LOADER const & loader)
		{
			// search in json for
			//
			// "object_names": {...} -> read all entries inside as "resource_name":data
			// or
			// "object_names": [...] -> read all entries inside as anonymous data
			//
			if (nlohmann::json const* objects_json = JSONTools::GetStructureNode(config, object_names))
			{
				for (nlohmann::json::const_iterator it = objects_json->begin(); it != objects_json->end(); ++it)
				{
					LOADER other_loader = loader; // use copy so that name and path store in loader is not applyed to wrong object
					if (objects_json->is_array())
						DoLoadObjectsFromConfiguration<RECURSE>(nullptr, *it, other_loader);
					else if (objects_json->is_object())
						DoLoadObjectsFromConfiguration<RECURSE>(it.key().c_str(), *it, other_loader);
				}
			}
			return true;
		}


		/** utility function to remove an object from a list */
		template<typename T, typename FUNC>
		static void DoRemoveObject(size_t index, T& vector, FUNC remove_func)
		{
			// ensure the index is valid
			size_t count = vector.size();
			if (index >= count)
				return;
			// copy the intrusive_ptr to prevent the destruction
			auto object = vector[index];
			// remove the object from the array
			if (index != count - 1)
				vector[index] = vector[count - 1];
			vector.pop_back();
			// callback then let the unreferencement manage the object lifetime
			remove_func(object.get());
		}

		/** detach all elements from a list */
		template<typename T, typename FUNC>
		static void RemoveAllObjectsFromList(T& vector, FUNC remove_func)
		{
			while (vector.size() > 0)
			{
				// copy the intrusive_ptr to prevent the destruction
				auto object = vector.back();
				// remove the object from the array
				vector.pop_back();
				// callback then let the unreferencement manage the object lifetime
				remove_func(object.get());
			}
		}

		template<typename T, typename U>
		static size_t FindObjectIndexInVector(T* object, U const& vector)
		{
			assert(object != nullptr);
			size_t count = vector.size();
			for (size_t i = 0; i < count; ++i)
				if (vector[i].get() == object)
					return i;
			return count;
		}

		/** utility function to test whether an object can be inserted */
		bool CanAddObject(ObjectRequest request, std::function<bool(ObjectRequest)> can_add_func) const;

	protected:

		/** whether the manager is started */
		bool manager_started = false;
	};

#endif

}; // namespace chaos