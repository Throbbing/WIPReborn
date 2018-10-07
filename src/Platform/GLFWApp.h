#pragma once
#include "../Logger.h"
#include "../AppManager.h"
#include "glfw_callbacks.h"
#include <vector>
#include "GlfwImguiRender.h"

#define NOCONSOLE #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#define MAKE_APP(class_name) \
int main(int argc, char** argv)\
{\
	GLFWApp* app = new class_name();\
	g_app = app;\
	app->init();\
	app->run();\
	delete app;\
	return 0;\
}\

class WIPCamera;
class WIPSprite;

extern void regist_user_component();

class GLFWApp : public WIPApp
{
public:
  GLFWApp() :window(nullptr), _exit_requist(false){}
  ~GLFWApp();

  virtual bool init();

  virtual void run();

  bool init_gl(const char *title, int width, int height);
  bool create_window(const char *title)
  {
    glfwSetErrorCallback(glfw_error_callback);
    if (!init_gl(title, window_w, window_h))
    {
      LOG_ERROR("init_gl failed!\n");
      return false;
    }
    glfwSetCursorPosCallback(window, glfw_mouse_pos_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwGetWindowSize(window, &window_w, &window_h);
    return true;
  }
  inline void require_exit()
  {
    _exit_requist = true;
  }
  GLFWwindow *window;
  f32 get_cur_time() const;

protected:


  bool _exit_requist;
  class TimeSource* times;
  class RBClock* clock;
  class RBTimerBase *timer;
  double lastTime;
  double curTime;
  float _frame;

  bool debug;

  GlfwImguiRender* imgui_renderer;

};

extern GLFWApp* g_app;