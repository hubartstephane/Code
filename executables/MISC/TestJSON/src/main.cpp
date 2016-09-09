
#include <chaos/LuaTools.h>
#include <chaos/WinTools.h>
#include <chaos/StringTools.h>
#include <chaos/FileTools.h>
#include <chaos/Application.h>


int _tmain(int argc, char ** argv, char ** env)
{
  chaos::Application::Initialize(argc, argv, env);

  chaos::WinTools::AllocConsoleAndRedirectStdOutput();

  boost::filesystem::path const & p = chaos::Application::GetInstance()->GetResourcesPath();


  nlohmann::json js;


 


  chaos::WinTools::PressToContinue();

  chaos::Application::Finalize();

  return 0;
}


