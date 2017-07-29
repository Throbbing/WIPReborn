#ifndef __SCROLLERWIDGET_H__
#define __SCROLLERWIDGET_H__

#include "BaseWidget.h"
class WIPTexture;
class ofApp;

class ScrollerWidget : public BaseWidget
{
public:
	ScrollerWidget();
	~ScrollerWidget();
	void update_button(int,int);
	void update(float dt);
	void set_position(int __x,int __y);

	virtual void draw_internal(int x=-1,int y=-1);
	void draw(int x=-1,int y=-1);
	bool init(const char* path_bar="resources/UI/loadingbar.png",const char* path_controll="resources/UI/controller.png");

	inline void set_mouse_state(int mx,int my,int _state)
	{
		_mouse_state = _state;
		_mx = mx;
		_my = my;
	}

	void reshape(int width,int height,int x,int y);

	void set_current_val(float val);

	float get_current_val();

	void set_controll(bool val);

	//editor
	virtual void update_editor();


protected:

private:
	WIPTexture* _bar;
	WIPTexture* _controll;
	int _gw;
	int _gh;
	int _mouse_state;
	int _button_state;
	int _mx;
	int _my;
	int _last_mouse_state;

	int _controll_current_x;
	int _controll_current_y;

	float _current_val;
	bool _bunder_controll;
};

#endif