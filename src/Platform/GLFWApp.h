#pragma once
#include "../Logger.h"
#include "../AppManager.h"
#include "glfw_callbacks.h"
#include <vector>
#include "GlfwImguiRender.h"
//#include "../thirdpart/glad/include/glad/glad.h"

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

  void load_rpg_demo();
  void load_tank_demo();
  void load_avg_demo();

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
    glfwGetWindowSize(window, &window_w,&window_h);
    return true;
  }
  inline void require_exit()
  {
	  _exit_requist = true;
  }
  GLFWwindow *window;
  f32 get_cur_time() const;
  class WorldRender* world_renderer;
private:


	bool _exit_requist;
	class TimeSource* times;
	class RBClock* clock;
	class RBTimerBase *timer;
	double lastTime;
	double curTime;
	float _frame;


	


#define ANIMYNUM 1
	WIPSprite* enemy[ANIMYNUM];
	WIPSprite* block;
	WIPSprite* player;
	WIPSprite* pop[ANIMYNUM];
	WIPSprite* bullets[ANIMYNUM];



	bool debug;


  
  std::vector<std::wstring> resw;
  int scoller_y;
  std::vector<class WIPTexture2D*> textures;
  std::vector<std::wstring> paths;


  GlfwImguiRender* imgui_renderer;
  class WIPLevelLoader* loader;
};

extern GLFWApp* g_app;