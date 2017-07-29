#ifndef __BASEWIDGET_H__
#define __BASEWIDGET_H__
#include "ActiveDrawableRect.h"

class BasePanel;
class BaseWidget : public ActiveDrawableRect
{
public:
	BaseWidget();

	void update(float dt);

	virtual void draw_internal(int x=-1,int y=-1);

	void draw(int x=-1,int y=-1);

private:
	/*
	int _x,_y;
	int _width,_height;
	*/
	//ofFbo _fbo;
};

#endif