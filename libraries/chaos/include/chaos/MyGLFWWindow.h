#pragma once

#include <chaos/StandardHeaders.h>

namespace chaos
{

  /**
   * MyGLFWWindowHints : this represents hint for GLFWwindow creation
   */

class MyGLFWWindowHints
{
public:

  /** constructor */
  MyGLFWWindowHints();
  /** gives set hints to GLFW */
  void ApplyHints();

public:

  /** true if we use an opengl debug context */
  int debug_context;
  /** the major version of opengl */
  int major_version;
  /** the major version of opengl */
  int minor_version;
  /** the refresh rate (only usefull in fullscreen mode) */
  int refresh_rate;
  /** the opengl profile */
  int opengl_profile;
  /** true if the window can be resized */
  int resizable;
  /** true if the window starts visible */
  int start_visible;
  /** true if the window has some decoration */
  int decorated;
  /** true if the window is toplevel */
  int toplevel;
  /** number of samples in multisamples (0 for none) */
  int samples;
  /** self description */
  int double_buffer;
  /** self description */
  int depth_bits;
  /** self description */
  int stencil_bits;
  /** self description */
  int red_bits;
  /** self description */
  int green_bits;
  /** self description */
  int blue_bits;
  /** self description */
  int alpha_bits;
  /** self description */
  int focused;
};

  /**
   * MyGLFWSingleWindowApplicationParams : parameters for playing single window application
   */

class MyGLFWSingleWindowApplicationParams
{
public:

  /** constructor */
  MyGLFWSingleWindowApplicationParams();

  /** the title */
  char const * title;
  /** the wanted monitor */
  GLFWmonitor * monitor;
  /** the monitor index */
  int monitor_index;
  /** window width */
  int width;
  /** window height */
  int height;
  /** the hints */
  MyGLFWWindowHints hints;
};

  /**
   * MyGLFWWindow : a binding class between chaos and GLFW to handle window (beware the prefix "My")
   */

class MyGLFWWindow 
{
public:

  /** constructor */
  MyGLFWWindow() : glfw_window(nullptr), refresh_required(false), double_buffer(true){}
  /** create and run single screen application (if width == height == 0 use pseudo fullscreen mode) */
  template<typename WINDOW_TYPE>
  static bool RunSingleWindowApplication(MyGLFWSingleWindowApplicationParams const & params)
  {
    bool result = false;

    WINDOW_TYPE * my_window = new WINDOW_TYPE();
    if (my_window != nullptr)
    {
      result = my_window->DoRunSingleWindowApplication(params); 
      delete(my_window);
    }
    return result;
  }

  /** get the hints for new GLFW window */
  virtual void TweakSingleWindowApplicationHints(MyGLFWWindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const;

  /** Get a list of all monitors sorted left to right */
  static std::vector<GLFWmonitor *> GetSortedMonitors();
  /** Get monitor by its index (negative for LEFT, positive for RIGHT, 0 for primary */
  static GLFWmonitor * GetMonitorByIndex(int monitor_index);

protected:

  /** an error callback */
  static void OnError(int code, const char* msg);
  /** the main function for single window application */
  virtual bool DoRunSingleWindowApplication(MyGLFWSingleWindowApplicationParams params);
  /** the main loop for single window application */
  virtual void DoRunSingleWindowMainLoop(GLFWwindow * glfw_window);
  /** bind MyGLFWWindow with GLFW */
  virtual void BindGLFWWindow(GLFWwindow * in_glfw_window);
  /** called every Tick (returns true whenever we want to redraw the window) */
  virtual bool Tick(double delta_time){ return true; }
  /** called at window creation (returns false if the window must be killed) */
  virtual bool Initialize(){ return true; }
  /** called at window destruction */
  virtual void Finalize(){ }

  /** called whenever the user try to close window */
  virtual bool OnWindowClosed(){ return true;}
  /** called whenever the window is resized */
  virtual void OnWindowResize(int width, int height){}
  /** called whenever the window is redrawn */
  virtual bool OnDraw(int width, int height){ return true; }
  /** called whenever the mouse is moved */ 
  virtual void OnMouseMove(double x, double y){}
  /** called whenever the mouse button is down / up */
  virtual void OnMouseButton(int button, int action, int modifier){}
  /** called whenever the mouse wheel is changed */
  virtual void OnMouseWheel(double scroll_x, double scroll_y){}
  /** called whenever a key is pressed */
  virtual void OnKeyEvent(int key, int scan_code, int action, int modifier){}
  /** called whenever a file is dropped */
  virtual void OnDropFile(int count, char const ** paths){}
  /** called whenever the window becomes iconified or is restored */
  virtual void OnIconifiedStateChange(bool iconified){}  
  /** called whenever the window gain or loose focus */
  virtual void OnFocusStateChange(bool gain_focus){}  

  /** called to require the window to close */
  void RequireWindowClosure();
  /** called to require the window to refresh */
  void RequireWindowRefresh();

private:

  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnWindowClosed(GLFWwindow * in_glfw_window);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnWindowResize(GLFWwindow * in_glfw_window, int width, int height);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnDraw(GLFWwindow * in_glfw_window);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnMouseMove(GLFWwindow * in_glfw_window, double x, double y);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnMouseButton(GLFWwindow * in_glfw_window, int button, int action, int modifier);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnMouseWheel(GLFWwindow * in_glfw_window, double scroll_x, double scroll_y);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnKeyEvent(GLFWwindow * in_glfw_window, int key, int scan_code, int action, int modifier);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnDropFile(GLFWwindow * in_glfw_window, int count, char const ** paths);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnIconifiedStateChange(GLFWwindow * in_glfw_window, int iconified);
  /** binding function between GLFW library and MyGLFWWindow */
  static void DoOnFocusStateChange(GLFWwindow * in_glfw_window, int gain_focus);  

protected:

  /** the window in GLFW library */
  GLFWwindow * glfw_window;
  /** is a refresh required */
  bool refresh_required;
  /** is the window with double buffer */
  bool double_buffer;
};

}; // namespace chaos

