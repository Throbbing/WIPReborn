#pragma once
#include "../Logger.h"
#include "../AppManager.h"
#include "glfw_callbacks.h"
#include <vector>
//#include "../thirdpart/glad/include/glad/glad.h"

class WIPCamera;
class WIPSprite;



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
    glfwGetWindowSize(window, &window_w,&window_h);
    return true;
  }
  inline void require_exit()
  {
	  _exit_requist = true;
  }
  GLFWwindow *window;

private:
	bool _exit_requist;
	class TimeSource* times;
	class RBClock* clock;
	class RBTimerBase *timer;
	double lastTime;
	double curTime;
	float _frame;
	class WIPTexture2D* tex2d;
	class WIPTexture2D* tex2d1;
	class WIPTexture2D* tex2d1mask;
	class WIPTexture2D* tex2d_fog;
	class WIPTexture2D* tex2d_lixiaoyao;
	class WIPTexture2D* tex2d_zaji1;
	class WIPTexture2D* tex2d_zaji2;
	class WIPTexture2D* tex2d_crowd;
	class WIPTexture2D* face;

	class WIPAnimationClip* clip;
	class WIPAnimationClip* clip1;
	class WIPAnimationClip* clip2;
	class WIPAnimationClip* clip3;
	class WIPAnimationClip* clip_s;
	class WIPAnimationClip* clip1_s;
	class WIPAnimationClip* clip2_s;
	class WIPAnimationClip* clip3_s;
	class WIPAnimationClip* pre_clip;

	class WorldRender* world_renderer;
	class TextRender* text_renderer;
	class UIRender* ui_renderer;

	int draw_state;

	std::vector<int> get_ids;
	std::vector<const WIPSprite*> get_objects;

	class WIPQuadTree* quad_tree;
	bool debug;


  float zoom;

  std::vector<WIPCamera*> cameras;
  class WIPScene* scene;
  std::vector<std::wstring> resw;
  WIPSprite* bg;
  WIPSprite* bg_mask;
  WIPSprite* man;
  WIPSprite* man_lixiaoyao;
  WIPSprite* zaji1;
  WIPSprite* zaji2;
  WIPSprite* crowd;
  WIPSprite* fogs;
  int scoller_y;

  std::vector<WIPTexture2D*> textures;
  std::vector<std::wstring> paths;
};