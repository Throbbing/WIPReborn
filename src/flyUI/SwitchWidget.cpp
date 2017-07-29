#include "SwitchWidget.h"

SwitchWidget::SwitchWidget()
{
	//ButtonWidget::ButtonWidget();
	_switch_state = false;

}

void SwitchWidget::init_button(std::string pic_path_down,std::string pic_path_move,std::string pic_path_normal,void (*button_event)(ButtonWidget*))
{
	ButtonWidget::init_button(pic_path_down,pic_path_move,pic_path_normal,button_event);
	_button_state = 1;
}

void SwitchWidget::update_button(int x,int y)
{
	_mx = x;
	_my = y;



	if(_mx>=_x&&_mx<=_x+_gw&&_my>=_y&&_my<=_y+_gh)
	{
		/*鼠标状态 _mouse_state:
		 *0：已经弹起；
		 *1：正在按下；
		 *2：正在弹起
		 *3：已经按下
		 ------------
		 *按钮状态 _button_state:
		 *0:unable
		 *1:off
		 *2:on
		 */
		if(_mouse_state==1)//pressing
		{
			_button_state = 2;//down
			_mouse_state = 3;//pressed
		}
		else if(_mouse_state==2)
		{

			_switch_state = !_switch_state;
			//_button_state = _switch_state ? 2 : 1;
			//_button_state = 1;//move
			if(_button_event)
				_button_event(this);
			_mouse_state = 0;
		}
		/*
		else
			if(_mouse_state!=3)
				_button_state = 1;
		*/
	}
	else
	{
		//_button_state = 0;
		_mouse_state = 0;
	}
	_button_state = _switch_state ? 2 : 1;
	if(!_bactive)
	{
		//_mouse_state = 0;
		_button_state = 0;
	}


}

void SwitchWidget::draw_internal(int x,int y)
{
	ButtonWidget::draw_internal(x,y);
}

void SwitchWidget::set_switch_state(bool val)
{
	_switch_state = val;
	_button_state = val ? 2 : 1;
}

bool SwitchWidget::get_switch_state()
{
	return _switch_state;
}