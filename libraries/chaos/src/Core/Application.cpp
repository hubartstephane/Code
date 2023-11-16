#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	Application * Application::singleton_instance = nullptr;

	Application::Application()
	{
		assert(singleton_instance == nullptr);
		singleton_instance = this; // well this should be a singleton
	}

	Application::~Application()
	{
		assert(singleton_instance == this);
		singleton_instance = nullptr;
	}

	void Application::LogExecutionInformation()
	{
		// display the options
#if _DEBUG
		Log::BeginTransaction(LogType::Message);
		Log::TransactionConcatLN("==========================================");
		Log::TransactionConcatLN("== Existing Options");
		Log::TransactionConcatLN("==========================================");
		Log::TransactionConcatLN(GlobalVariableManager::GetInstance()->GetOptionString());
		Log::EndTransaction();
#endif
		// display the arguments
		Log::BeginTransaction(LogType::Message);
		Log::TransactionConcatLN("==========================================");
		Log::TransactionConcatLN("== Command line");
		Log::TransactionConcatLN("==========================================");
		for (std::string const& arg : GetArguments())
			Log::TransactionConcatLN("%s", arg.c_str());
		Log::EndTransaction();

		// display some informations
		Log::BeginTransaction(LogType::Message);
		Log::TransactionConcatLN("==========================================");
		Log::TransactionConcatLN("== Informations");
		Log::TransactionConcatLN("==========================================");
		Log::TransactionConcatLN("Working directory: %s", boost::filesystem::current_path().string().c_str());
		Log::TransactionConcatLN("%s", StringTools::TimeToString(std::chrono::system_clock::now(), TimeToStringFormatType::FULL).c_str());

		Log::EndTransaction();
	}

	bool Application::ReloadConfigurationFile()
	{
		return ReloadObjectConfiguration(true); // send notifications
	}

	boost::filesystem::path Application::GetConfigurationPath() const
	{
		return GetResourcesPath() / "config.json";
	}

	bool Application::InitializeConfigurationSystem()
	{
		if (RootObjectConfiguration* root_config = new RootObjectConfiguration())
		{
			root_config->SetReadConfigPath(GetConfigurationPath());
			root_config->SetPersistentConfigPath(GetPersistentConfigurationPath());
			root_config->LoadConfiguration(true, true, false); // load read and persistent data. do not send notifications
			SetObjectConfiguration(root_config);
			return true;
		}
		return true;
	}

	bool Application::SavePersistentConfiguration() const
	{
		if (RootObjectConfiguration* root_config = auto_cast(GetObjectConfiguration()))
			return root_config.SavePersistentConfiguration();
		return false;
	}
	
	boost::filesystem::path Application::GetPersistentConfigurationPath() const
	{
		return GetUserLocalTempPath() / "persistent_config.json";
	}

	bool Application::LoadClasses()
	{
		if (nlohmann::json const* classes_json = JSONTools::GetObjectNode(GetJSONReadConfiguration(), "classes"))
		{
			std::string classes_directory;
			if (JSONTools::GetAttribute(*classes_json, "classes_directory", classes_directory))
			{
				ClassLoader loader;
				loader.LoadClassesInDirectory(ClassManager::GetDefaultInstance(), classes_directory);
			}
		}
		return true;
	}

	namespace GlobalVariables
	{
		CHAOS_GLOBAL_VARIABLE(bool, ShowConsole, false);
		CHAOS_GLOBAL_VARIABLE(bool, HideConsole, false);
		CHAOS_GLOBAL_VARIABLE(bool, DumpConfigFile, false);
		CHAOS_GLOBAL_VARIABLE(bool, ShowDirectories, false);
		CHAOS_GLOBAL_VARIABLE(bool, ShowUserTempDirectory, false);
		CHAOS_GLOBAL_VARIABLE(bool, ShowInstalledResourcesDirectory, false);
	};

	bool Application::Initialize()
	{
		// prepare the configuration (do not apply any data to application yet)
		InitializeConfigurationSystem();

		// prepare the logger
		if (Logger* logger = Logger::GetInstance())
			if (LoggerListener* listener = new FileLoggerListener())
				listener->SetLogger(logger);

		HelpText::OutputToLogs();

		// show console
		bool will_show_console = show_console;
		if (GlobalVariables::ShowConsole.Get())
			will_show_console = true;
		else if (GlobalVariables::HideConsole.Get())
			will_show_console = false;

		will_show_console = false;

		if (will_show_console)
			WinTools::AllocConsoleAndRedirectStdOutput();

		// some log
		LogExecutionInformation();

		// load class
		if (!LoadClasses())
			return false;

		// initialize the managers
		if (!InitializeManagers())
			return false;
		// open user temp directory and dump the config file
		boost::filesystem::path user_temp = GetUserLocalPath(); // XXX : this directory is necessary for some per application data
#if _DEBUG
		// display the directories to help debugging
		bool dump_config = GlobalVariables::DumpConfigFile.Get();
		if (dump_config)
			JSONTools::DumpConfigFile(configuration);
		if (dump_config || GlobalVariables::ShowDirectories.Get() || GlobalVariables::ShowUserTempDirectory.Get())
			WinTools::ShowFile(user_temp);
		if (GlobalVariables::ShowDirectories.Get() || GlobalVariables::ShowInstalledResourcesDirectory.Get())
			WinTools::ShowFile(GetResourcesPath());
#endif

		// read the configurable properties
		ReadConfigurableProperties();

		return true;
	}

	void Application::Finalize()
	{
		FinalizeManagers();
	}

	int Application::Main()
	{
		return 0;
	}

	int Application::Run(int argc, char ** argv, char ** env)
	{
		bool result = false;
		if (InitializeStandardLibraries())
		{
			// parse the parameters
			GlobalVariableManager::GetInstance()->ParseArguments(argc, argv);

			// store a copy of the parameters
			StoreParameters(argc, argv, env);
			// create a user temp directory if necessary */
			CreateUserLocalTempDirectory();
			// load the configuration file (ignore return value because there is no obligation to use a configuration file)
			InitializeConfigurationSystem();
			// initialize, run, and finalize the application
			if (Initialize())
			{
				result = Main();
				StorePersistentProperties();
				SavePersistentConfiguration();
			}
			// finalization (even if initialization failed)
			Finalize();
			FinalizeStandardLibraries();
		}
		return result;
	}

	bool Application::InitializeStandardLibraries()
	{
		return true;
	}

	void Application::FinalizeStandardLibraries()
	{
	}

	bool Application::InitializeManagers()
	{
		return true;
	}

	void Application::FinalizeManagers()
	{
	}

	void Application::StoreParameters(int argc, char ** argv, char ** env)
	{
		assert(argc > 0); // should at least contains the fullpath of the application

		if (argv != nullptr)
			for (int i = 0; i < argc; ++i)
				arguments.push_back(argv[i]);

		if (env != nullptr)
		{
			for (int i = 0; env[i] != nullptr; ++i)
			{
				char const * separator = strchr(env[i], '=');
				if (separator != nullptr)
				{
					std::string key = std::string(env[i], separator - env[i]);
					std::string value = std::string(separator + 1);
					environments.push_back(std::make_pair(key, value));
				}
			}
		}

		boost::filesystem::path p = argv[0];
		p = p.lexically_normal();
		p.make_preferred();

		application_filename = p.filename();
		application_name = application_filename.replace_extension().string();

		application_path = p.parent_path();
		resources_path = application_path / "resources";
		userlocal_path = WinTools::GetUserLocalPath() / application_name;
		userlocal_temp_path = userlocal_path / "temp";
	}

	char const* Application::GetApplicationName()
	{
		Application const* application = Application::GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetName();
	}

	char const * Application::GetApplicationEnvironment(char const * key)
	{
		Application const * application = Application::GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetEnvironment(key);
	}

	char const * Application::GetEnvironment(char const * key) const
	{
		assert(key != nullptr);
		for (auto e : environments)
			if (StringTools::Stricmp(e.first, key) == 0)
				return e.second.c_str();
		return nullptr;
	}

	boost::filesystem::path const & Application::CreateUserLocalTempDirectory() const
	{
		boost::filesystem::path const & result = GetUserLocalTempPath();
		if (!boost::filesystem::is_directory(result))
			boost::filesystem::create_directories(result);
		return result;
	}

	InputMode Application::GetApplicationInputMode()
	{
		Application const * application = Application::GetConstInstance();
		if (application != nullptr)
			return application->GetInputMode();
		return InputMode::KEYBOARD;
	}

	void Application::SetApplicationInputMode(InputMode new_mode)
	{
		Application * application = Application::GetInstance();
		if (application != nullptr)
			application->SetInputMode(new_mode);
	}

	bool Application::HasApplicationCommandLineFlag(char const * flag_name)
	{
		Application const * application = Application::GetInstance();
		if (application == nullptr)
			return false;
		return application->HasCommandLineFlag(flag_name);
	}

	bool Application::HasCommandLineFlag(char const * flag_name) const
	{
		assert(flag_name != nullptr);
		for (std::string const & arg : arguments)
			if (StringTools::Stricmp(arg, flag_name) == 0)
				return true;
		return false;
	}

#if _DEBUG
	void Application::SetFileRedirectionDirectories(boost::filesystem::path const & build_path, std::string const & direct_access_paths)
	{
		redirection_build_path = build_path.lexically_normal();

		std::vector<std::string> extra = StringTools::Split(direct_access_paths.c_str(), ';');
		for (std::string & e : extra)
		{
			boost::filesystem::path p = std::move(e);
			if (!p.empty())
				redirection_source_paths.push_back(p.lexically_normal());
		}
	}
#endif // _DEBUG

}; // namespace chaos
