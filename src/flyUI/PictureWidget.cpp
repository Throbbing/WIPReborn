#include "PictureWidget.h"
#include "../wip/Renderer.h"

PictureWidget::PictureWidget(int x,int y,int width,int height)
{
	_x = x;
	_y = y;
	_width = width;
	_height = height;
	_x_space = 15;
	
	_binit = false;

}

PictureWidget::~PictureWidget()
{
	delete _image;
}

void PictureWidget::static_init(std::string image_path)
{

	_image = new WIPTexture(image_path.c_str());


	_binit = true;

	_fbo->resize(_width,_height);
	draw_internal();

}

void PictureWidget::set_x_space(int xs)
{
	_x_space = xs;
}

void PictureWidget::reset_pic(const char* path)
{
	delete _image;
	_image = new WIPTexture(path);
	draw_internal();
	/*
	if(!_fbo.isAllocated())
		return;
	_fbo.begin();
	ofClear(0,0,0,0);
	_font.drawString(_str,_x_space,_height - (_height-_line_height)/2.f);
	_fbo.end();
	*/
}

void PictureWidget::draw_internal(int x,int y)
{

	_fbo->begin();
	_fbo->clear();
	if(_image)
		g_renderer->render(_image->get_texture_id(),0,0,_fbo->get_width(),_fbo->get_height());
	_fbo->end();
}

void PictureWidget::draw(int x,int y)
{
	//BaseWidget::draw(ofGetMouseX(),ofGetMouseY());
	BaseWidget::draw(x,y);
}

void PictureWidget::update(float dt)
{
	ActiveDrawableRect::update(dt);
}

void PictureWidget::reshape(int width,int height,int x,int y)
{
	ActiveDrawableRect::reshape(width,height,x,y);
	draw_internal();
}