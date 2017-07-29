#ifndef __PICTUREWIDGET_H__
#define __PICTUREWIDGET_H__

#include "../wip/Texture.h"
#include "BaseWidget.h"
//static
class PictureWidget :public BaseWidget
{
public:
	PictureWidget(int x,int y,int width=60,int height=20);
	~PictureWidget();

	void static_init(std::string image_path="");

	void update(float dt);
	
	void draw_internal(int x=-1,int y=-1);
	
	void draw(int x=-1,int y=-1);

	void set_x_space(int xs);

	void reset_pic(const char* path);

	void set_position(int x,int y)
	{
		_x = x;
		_y = y;
		draw_internal();
	}

	WIPTexture* get_image()
	{
		return _image;
	}

	virtual void reshape(int width,int height,int x=-1,int y=-1);

protected:

private:
	std::string _str;
	WIPTexture* _image;
	//ofTrueTypeFont  _font;
	float _line_height,_letter_space;
	//x space from head
	int _x_space;
	int _font_size;
	bool _binit;
	
};

#endif