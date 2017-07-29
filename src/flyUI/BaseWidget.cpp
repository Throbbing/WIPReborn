#include "BaseWidget.h"
#include "../wip/Renderer.h"

BaseWidget::BaseWidget()
{

}

void BaseWidget::update(float dt)
{
	ActiveDrawableRect::update(dt);
	//std::cout<<"updating BaseWidget "<<_name<<" ...\n";
}

void BaseWidget::draw(int x,int y)
{
	if(x<0||y<0)
		g_renderer->render(_fbo->get_texture_id(),_x,_y,_width,_height);
	else
		g_renderer->render(_fbo->get_texture_id(),x,y,_width,_height);
}

void BaseWidget::draw_internal(int x,int y)
{

}