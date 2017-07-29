#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "BaseWidget.h"
#include <string>
#include "../wip/Texture.h"
class ofApp;

template<typename T>
class Callbacker
{
public:
	
	
};


//TODO£º×¢²á»Øµ÷º¯Êý
class ButtonWidget : public BaseWidget
{
public:
	ButtonWidget();
	~ButtonWidget();
	void init_button(std::string pic_path_down,std::string pic_path_move,std::string pic_path_normal,void (*button_event)(ButtonWidget*));
	virtual void update_button(int,int);
	//void draw();
	void set_position(int __x,int __y);
	inline void set_mouse_state(int mx,int my,int _state)
	{
		_mouse_state = _state;
		_mx = mx;
		_my = my;
	}


	void update(float dt);

	void draw_internal(int x=-1,int y=-1);

	void draw(int x=-1,int y=-1);

	void set_size(int width,int height);

	void set_callback_holder(ofApp* holder);

	ofApp* get_callback_holder();

	bool call;
protected:
	bool _binit;
	WIPTexture* _image_down;
	WIPTexture* _image_move;
	WIPTexture* _image_normal;
	void (*_button_event)(ButtonWidget*);
	int _gw;
	int _gh;
	int _mouse_state;
	int _button_state;
	int _mx;
	int _my;
	int _last_mouse_state;

	//callback
	ofApp *_callback_holder;

private:
	
};

#endif