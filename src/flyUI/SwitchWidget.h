#ifndef __SWITCHWIDGET_H__
#define __SWITCHWIDGET_H__
#include "ButtonWidget.h"

class SwitchWidget : public ButtonWidget
{
public:
	SwitchWidget();
	void init_button(std::string pic_path_down,std::string pic_path_move,std::string pic_path_normal,void (*button_event)(ButtonWidget*));
	void update_button(int x,int y);
	void draw_internal(int x/* =-1 */,int y/* =-1 */);
	//void update(float dt);

	void set_switch_state(bool val);
	bool get_switch_state();
	
protected:

private:
	//if on ro off
	//on - down
	//off - move
	//unable - normal
	bool _switch_state;
};

#endif