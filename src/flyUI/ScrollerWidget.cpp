#include "ScrollerWidget.h"
#include "../wip/Texture.h"
#include "../wip/Input.h"
#include "../wip/Renderer.h"

ScrollerWidget::ScrollerWidget()
{
	_controll = _bar = nullptr;
	_x = _y = 0;
	_bunder_controll = true;
}

ScrollerWidget::~ScrollerWidget()
{
	delete _controll;
	delete _bar;
}

void ScrollerWidget::set_position(int __x,int __y)
{
	_x = __x;
	_y = __y;
	_controll_current_x = _x + _current_val * _bar->get_width();
	_controll_current_y = _y;
}

bool ScrollerWidget::init(const char* path_bar,const char* path_controll)
{
	_controll = new WIPTexture(path_controll);
	_bar = new WIPTexture(path_bar);
	_width = _bar->get_width() + 15;
	_height = _controll->get_height()*1.5;
	_fbo->resize(_width,_height);
	_mouse_state = 0;//normal
	_button_state = 0;//normal
	_last_mouse_state = 0;
	_controll_current_x = _controll_current_y = 0;
	_gw = _controll->get_width() + 5;
	_gh = _controll->get_height() + 5;
	_current_val = 0;

	return true;
}

void ScrollerWidget::update_button(int __x,int __y)
{
	if(!_bunder_controll)
		return;
	_mx = __x;
	_my = __y;
	//static bool _bpressed = false;

	if(_mx>=_controll_current_x&&_mx<=_controll_current_x+_gw&&_my>=_controll_current_y&&_my<=_controll_current_y+_gh)
	{
		/*鼠标状态 _mouse_state:
		 *0：已经弹起；
		 *1：正在按下；
		 *2：正在弹起
		 *3：已经按下
		 ------------
		 *按钮状态 _button_state:
		 *0:Nomal
		 *1:Move
		 *2:Down
		 */


		
		if(_mouse_state==1)//pressing
		{
			_button_state = 2;//down
			_mouse_state = 3;//pressed
		}
		else if(_mouse_state==2)
		{
			//if(_button_event)
				//_button_event(this);
			_button_state = 1;//move
			_mouse_state = 0;
		}
		else
		{
			if(_mouse_state==3&&_button_state==2)
			{
				float t = _mx - _controll->get_width()/2;
				if(t-_x>=0&&t-_x<=_bar->get_width())
				{
					_controll_current_x = t;
					_current_val = (t-_x)/_bar->get_width();
				}
			}
			if(_mouse_state!=3)
				_button_state = 1;
		}
	}
	else
	{
		_button_state = 0;
		_mouse_state = 0;
		
	}
	
	

}

void ScrollerWidget::update(float dt)
{
	if(!_bactive)
		return;
	ActiveDrawableRect::update(dt);
	int mouse_x = Input::get_mouse_x();
	int mouse_y = Input::get_mouse_y();
	//if left button is pressed state.Ture for pressed.
	bool bpressed = Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON);
	if(_last_mouse_state==0&&bpressed==1)
		set_mouse_state(mouse_x,mouse_y,1 );
	else if(_last_mouse_state==1&&bpressed==0)
		set_mouse_state(mouse_x,mouse_y,2 );
	update_button(mouse_x,mouse_y);
	//set_mouse_state(mouse_x,mouse_y,bpressed? 1 : 2 );
	
	draw_internal();
	_last_mouse_state = bpressed;
}

void ScrollerWidget::draw(int x,int y)
{
	BaseWidget::draw(x,y);
}

void ScrollerWidget::draw_internal(int x,int y)
{
	_fbo->begin();
	_fbo->clear();

	g_renderer->render(_bar->get_texture_id(),0,0,_width,_bar->get_height()+3);
	g_renderer->render(_controll->get_texture_id(),_controll_current_x-_x-1,_controll_current_y-_y-13,_controll->get_width(),_controll->get_height());

	_fbo->end();
}

void ScrollerWidget::reshape(int x,int y, int t,int r)
{
	//cancle this method
	return;
}

void ScrollerWidget::set_current_val(float val)
{
	if(val>1) val = 1;
	else if(val<0) val = 0;
	_current_val = val;
	int t = _current_val * _bar->get_width();
	_controll_current_x = _x + t;
}

float ScrollerWidget::get_current_val()
{
	return _current_val;
}

void ScrollerWidget::update_editor()
{
	bool m = Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON);
	if(m&&!_last_mouse)
	{
		std::cout<<"("<<Input::get_mouse_x()<<","<<Input::get_mouse_y()<<")"<<std::endl;
		set_position(Input::get_mouse_x(),Input::get_mouse_y());
		cancel_mouse_position();
	}
	_last_mouse = m;
	
}

void ScrollerWidget::set_controll(bool val)
{
	_bunder_controll = val;
}
