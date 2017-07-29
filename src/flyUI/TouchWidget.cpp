#include "TouchWidget.h"

TouchWidget::TouchWidget(int x,int y,int width,int height)
{
	_x = x;
	_y = y;
	_width = width;
	_height = height;
}

void TouchWidget::update(float dt)
{
	ButtonWidget::update(dt);
	int lenw = _widgets.size();

	_fbo->begin();

	
	for(int i=0;i<lenw;++i)
	{
		_widgets[i]->update(dt);
		_widgets[i]->draw();
	}

	_fbo->end();
}

void TouchWidget::add_widget(BaseWidget* widget)
{

	_widgets.push_back(widget);
}

std::vector<BaseWidget*>& TouchWidget::get_contents()
{
	return _widgets;
}

BaseWidget* TouchWidget::get_widget_by_name(std::string name)
{
	int len = _widgets.size();
	for(int i=0;i<len;++i)
	{
		if(name==_widgets[i]->get_name())
			return _widgets[i];
	}
	return nullptr;
}