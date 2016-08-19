#include <chaos/StandardHeaders.h> 
#include <chaos/FileTools.h> 
#include <chaos/LogTools.h> 
#include <chaos/GLTools.h> 
#include <chaos/MyGLFWGamepadManager.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/GLProgramLoader.h>
#include <chaos/Application.h>
#include <chaos/SimpleMeshGenerator.h>
#include <chaos/SkyBoxTools.h>
#include <chaos/GLDebugOnScreenDisplay.h>
#include <chaos/MyGLFWFpsCamera.h>
#include <chaos/SimpleMesh.h>
#include <chaos/GLProgramData.h>
#include <chaos/GLProgram.h>
#include <chaos/Texture.h>
#include <chaos/VertexDeclaration.h>


bool IS_INSIDE_CUBE = true;
int  SKYBOX_CHOICE  = 0;

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFWWindow
{
public:

  MyGLFWWindowOpenGLTest1(){}

protected:

  virtual bool OnDraw(int width, int height) override
  {
    glm::vec4 clear_color(0.0f, 0.0f, 0.0f, 0.0f);
    glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

    float far_plane = 1000.0f;
    glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);   // when viewer is inside the cube
   
    glUseProgram(program->GetResourceID());

    // XXX : the scaling is used to avoid the near plane clipping      
    static float FOV =  60.0f;
    glm::mat4 projection_matrix      = glm::perspectiveFov(FOV * (float)M_PI / 180.0f,(float)width, (float)height, 1.0f, far_plane);
    glm::mat4 local_to_world_matrix  = glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
    glm::mat4 world_to_camera_matrix = fps_camera.GlobalToLocal();
      
    chaos::GLProgramData const & program_data = program->GetProgramData();

    program_data.SetUniform("projection",      projection_matrix);
    program_data.SetUniform("local_to_world",  local_to_world_matrix);
    program_data.SetUniform("world_to_camera", world_to_camera_matrix);

    glBindTextureUnit(0, texture->GetResourceID());
    program_data.SetUniform("material", 0);

    mesh->Render(program_data, nullptr, 0, 0);

    debug_display.Display(width, height);    

    return true;
  }

  virtual void Finalize() override
  {
    program = nullptr;
    mesh    = nullptr;
    texture = nullptr;

    skybox.Release(true);

    debug_display.Finalize();
  }

  virtual bool Initialize() override
  {   
    chaos::Application * application = chaos::Application::GetInstance();
    if (application == nullptr)
      return false;

    boost::filesystem::path resources_path = application->GetApplicationPath() / "resources";
    boost::filesystem::path image_path     = resources_path / "font.png";

    chaos::GLDebugOnScreenDisplay::Params debug_params;
    debug_params.texture_path               = image_path;
    debug_params.font_characters            = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    debug_params.font_characters_per_line   = 10;
    debug_params.font_characters_line_count = 10;
    debug_params.character_width            = 20;
    debug_params.spacing                    = glm::ivec2( 0, 0);
    debug_params.crop_texture               = glm::ivec2(15, 7);

    if (!debug_display.Initialize(debug_params))
      return false;

    debug_display.AddLine("HelloWorld");


    if (SKYBOX_CHOICE == 0)
      skybox = chaos::SkyBoxTools::LoadSingleSkyBox((resources_path / "violentdays_large.jpg").string().c_str());
    else if (SKYBOX_CHOICE == 1)
      skybox = chaos::SkyBoxTools::LoadSingleSkyBox((resources_path / "originalcubecross.png").string().c_str());
    else
    {
      boost::filesystem::path p = resources_path / "Maskonaive";

      boost::filesystem::path left_image   = p / "negx.jpg";  
      boost::filesystem::path front_image  = p / "posz.jpg";
      boost::filesystem::path right_image  = p / "posx.jpg";
      boost::filesystem::path back_image   = p / "negz.jpg";
      boost::filesystem::path top_image    = p / "posy.jpg";
      boost::filesystem::path bottom_image = p / "negy.jpg";

      skybox = chaos::SkyBoxTools::LoadMultipleSkyBox(
        left_image.string().c_str(), 
        right_image.string().c_str(), 
        top_image.string().c_str(), 
        bottom_image.string().c_str(), 
        front_image.string().c_str(), 
        back_image.string().c_str());
    }   

    if (skybox.IsEmpty())
      return false;
    
    texture = chaos::GLTools::GenTextureObject(&skybox);
    if (texture == nullptr)
      return false;

    chaos::GLProgramLoader loader;
    loader.AddShaderSourceFile(GL_FRAGMENT_SHADER, resources_path / "pixel_shader_cube.txt");
    loader.AddShaderSourceFile(GL_VERTEX_SHADER,   resources_path / "vertex_shader.txt");
    
    program = loader.GenerateProgramObject();
    if (program == nullptr)
      return false;

    chaos::box3 b = chaos::box3(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    mesh = chaos::CubeMeshGenerator(b).GenerateMesh(); 
    if (mesh == nullptr)
      return false;

    if (!IS_INSIDE_CUBE)
      fps_camera.fps_controller.position.z = 50.0f;
   
    return true;
  }

  virtual void TweakSingleWindowApplicationHints(chaos::MyGLFWWindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
  {
    chaos::MyGLFWWindow::TweakSingleWindowApplicationHints(hints, monitor, pseudo_fullscreen);

    hints.toplevel  = 1;
    hints.decorated = 1;
  }

  virtual bool Tick(double delta_time) override
  {
    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      RequireWindowClosure();

    fps_camera.Tick(glfw_window, delta_time);

    debug_display.Tick(delta_time);
    
    return true; // refresh
  }

  virtual void OnMouseButton(int button, int action, int modifier) override
  {
    if (button == 1 && action == GLFW_RELEASE)
      debug_display.AddLine("HelloWorld");

    fps_camera.OnMouseButton(glfw_window, button, action, modifier);
  }

  virtual void OnMouseMove(double x, double y) override
  {
    fps_camera.OnMouseMove(glfw_window, x, y);
  }

protected:

  boost::intrusive_ptr<chaos::GLProgram>  program;
  boost::intrusive_ptr<chaos::SimpleMesh> mesh;
  boost::intrusive_ptr<chaos::Texture>    texture;
  
  chaos::SkyBoxImages skybox;

  chaos::MyGLFWFpsCamera fps_camera;

  chaos::GLDebugOnScreenDisplay debug_display;
};

int _tmain(int argc, char ** argv, char ** env)
{
  chaos::Application::Initialize<chaos::Application>(argc, argv, env);

  chaos::WinTools::AllocConsoleAndRedirectStdOutput();
    
  FreeImage_Initialise(); // glew will be initialized 
  glfwInit();

  chaos::MyGLFWSingleWindowApplicationParams params;
  params.monitor       = nullptr;
  params.width         = 500;
  params.height        = 500;
  params.monitor_index = 0;
  chaos::MyGLFWWindow::RunSingleWindowApplication<MyGLFWWindowOpenGLTest1>(params);

  glfwTerminate();
  FreeImage_DeInitialise();

  return 0;
}


