#include "BaseWidget.h"
#include "BasePanel.h"
#include "..\wip\Renderer.h"

BasePanel::BasePanel(int x,int y,int width,int height)
{
	_x = x;
	_y = y;
	_width = width;
	_height = height;
	_background = NULL;
	
	//wrong with global construct
	//_fbo.allocate(width,height);
	_fbo->resize(width,height);
}

BasePanel::~BasePanel()
{
	delete _background;
}

void BasePanel::set_background(const char *path)
{
	if(!_background)
		_background = new WIPTexture(path);
}

void BasePanel::set_font(const char* path)
{
	printf("You set font,but this method is not in use\n");
	//_font.loadFont(path,14);
}

void BasePanel::add_panel(BasePanel* panel)
{

	_sub_panels.push_back(panel);
}

void BasePanel::add_widget(BaseWidget* widget)
{

	_widgets.push_back(widget);
}

void BasePanel::update(float dt)
{
	int tempnum = 0;

	ActiveDrawableRect::update(dt);
	int lenp = _sub_panels.size();
	int lenw = _widgets.size();

	_fbo->begin();

	_fbo->clear();
	
	g_renderer->render(_background->get_texture_id(),0,0,_width,_height);

	
	for(int i=0;i<lenp;++i)
	{
		_sub_panels[i]->update(dt);
		_sub_panels[i]->draw();
		//editor
		_contents[tempnum] = _sub_panels[i];
		tempnum++;
	}
	
	for(int i=0;i<lenw;++i)
	{
		_widgets[i]->update(dt);
		_widgets[i]->draw();
		//editor
		_contents[tempnum] = _widgets[i];
		tempnum++;
	}
	
	_all_num = tempnum;
	//std::cout<<"updating BasePanel "<<_name<<" ...\n";
	

	_fbo->end();
}

void BasePanel::draw(int x,int y,int width,int height)
{
	if(x<0||y<0)
		g_renderer->render(_fbo->get_texture_id(),_x,_y,_width,_height);
	else
		g_renderer->render(_fbo->get_texture_id(),x,y,width,height);
	
}

void BasePanel::draw_internal(int x,int y)
{

}