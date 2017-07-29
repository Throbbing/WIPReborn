#ifndef __BASEPANEL_H__
#define __BASEPANEL_H__
#include "../wip/Texture.h"
#include "define.h"
#include "ActiveDrawableRect.h"
//#include <vector>
class BaseWidget;

class BasePanel : public ActiveDrawableRect
{
public:
	BasePanel(int x,int y,int width,int height);
	~BasePanel();

	void set_background(const char* path);
	void set_font(const char* path);

	void add_panel(BasePanel* panel);
	void add_widget(BaseWidget* widget);

	void update(float dt);

	virtual void draw_internal(int x=-1,int y=-1);
	//if not specified the position,draw with _x,_y
	void draw(int x=-1,int y=-1,int width=0,int height=0);

protected:
	WIPTexture* _background;
	//ofTrueTypeFont _font;

	//ofFbo _fbo;

	std::vector<BasePanel*> _sub_panels;
	std::vector<BaseWidget*> _widgets;
private:
	/*
	int _x,_y;
	int _width,_height;
	*/


};

#endif