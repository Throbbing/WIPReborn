#ifndef __TOUCHPANEL_H__
#define __TOUCHPANEL_H__
#include "ButtonWidget.h"

class TouchWidget : public ButtonWidget
{
public:

	TouchWidget(int x,int y,int width,int height);	
	void update(float dt);

	void add_widget(BaseWidget* widget);
	std::vector<BaseWidget*>& get_contents();
	BaseWidget* get_widget_by_name(std::string name);
protected:

private:
	std::vector<BaseWidget*> _widgets;
};

#endif