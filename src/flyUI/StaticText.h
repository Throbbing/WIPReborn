/*
#ifndef __STATICTEXT_H__
#define __STATICTEXT_H__
#include "ofMain.h"
#include "BaseWidget.h"
//static
class StaticText :public BaseWidget
{
public:
	StaticText(int x,int y,std::string str="",int width=60,int height=20);

	void static_init(std::string font_path="", int font_size=28,float line_height=18.f,float letter_space=1.037f);

	void update(float dt);
	
	void draw_internal(int x=-1,int y=-1);
	
	void draw(int x=-1,int y=-1);

	void set_x_space(int xs);

	void reset_string(std::string str);

	void set_position(int x,int y)
	{
		_x = x;
		_y = y;
		draw_internal();
	}

	ofTrueTypeFont& get_font()
	{
		return _font;
	}

	virtual void reshape(int width,int height,int x=-1,int y=-1);

	string get_str();

	void set_font_color(ofColor color)
	{
		_font_color = color;
	}

protected:

private:
	std::string _str;
	ofTrueTypeFont  _font;
	float _line_height,_letter_space;
	//x space from head
	int _x_space;
	int _font_size;
	bool _binit;

	ofColor _font_color;
	
};

#endif
*/