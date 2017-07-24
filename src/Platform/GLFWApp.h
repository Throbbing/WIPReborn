#pragma once
#include "../Logger.h"
#include "../AppManager.h"
#include "glfw_callbacks.h"
#include <vector>
//#include "../thirdpart/glad/include/glad/glad.h"

class WIPCamera;

#define MAXN 100
#define MEMSIZE 1024*1024


enum class ManState
{
	E_LEFT,
	E_RIGHT,
	E_UP,E_DOWN
};

class GLFWApp : public WIPApp 
{
public:
	GLFWApp() :window(nullptr), _exit_requist(false){}
    ~GLFWApp();

  virtual bool init();

  virtual void run();

  inline int ofGetWindowHeight()
  {
    return window_h;
  }
  inline int ofGetWindowWidth()
  {
    return window_w;
  }
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
	

	class WIPSprite* sprites[MAXN];
	int draw_state;

	std::vector<int> get_ids;
	std::vector<const WIPSprite*> get_objects;

	class WIPQuadTree* quad_tree;
	bool debug;
  int pack_sprite(const WIPSprite **sprite, void *mem, int n,int offset_n);
  void pack_index(void *mem, int n);

  float zoom;

  std::vector<WIPCamera*> cameras;
  class WIPScene* scene;

  WIPSprite* bg;
  WIPSprite* bg_mask;
  WIPSprite* man;
  WIPSprite* man_lixiaoyao;
  WIPSprite* zaji1;
  WIPSprite* zaji2;
  WIPSprite* crowd;
  WIPSprite* fogs;

  ManState man_state;
};