
#include "StaticText.h"

StaticText::StaticText(int x,int y,std::string str,int width,int height)
{
	_x = x;
	_y = y;
	_width = width;
	_height = height;
	_str = str;
	_x_space = 15;
	
	_binit = false;

}

void StaticText::static_init(std::string font_path,int font_size,float line_height,float letter_space)
{
	_font_color = ofColor(255,255,255);
	_line_height = line_height;
	_font_size = font_size;
	_letter_space = letter_space;
	if(!_font.loadFont(font_path,_font_size))
		_font.loadFont("frabk.ttf",_font_size);
	_font.setLineHeight(_line_height);
	_font.setLetterSpacing(_letter_space);
	_binit = true;

	_fbo.allocate(_width,_height);
	draw_internal();

}

void StaticText::set_x_space(int xs)
{
	_x_space = xs;
}

void StaticText::reset_string(std::string str)
{
	_str = str;
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

void StaticText::draw_internal(int x,int y)
{
	if(!_fbo.isAllocated())
		return;
	_fbo.begin();
	ofClear(0,0,0,0);
	int y_space = _height - (_height-_line_height)/2.f;
	ofPushStyle();
	ofSetColor(_font_color);
	//anchor is on the left-button
	_font.drawString(_str,_x_space,y_space /*_height - (_height-_line_height)/2.f*/);
	ofPopStyle();
	_fbo.end();
}

void StaticText::draw(int x,int y)
{
	//BaseWidget::draw(ofGetMouseX(),ofGetMouseY());
	BaseWidget::draw(x,y);
}

void StaticText::update(float dt)
{
	ActiveDrawableRect::update(dt);
}

void StaticText::reshape(int width,int height,int x,int y)
{
	ActiveDrawableRect::reshape(width,height,x,y);
	draw_internal();
}

string StaticText::get_str()
{
	return _str;
}