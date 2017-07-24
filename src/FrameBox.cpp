#include "FrameBox.h"


WIPFrameBox::WIPFrameBox()
{
	lt = RBVector2(0.f,1.f);
	lb = RBVector2(0.f,0.f);
	rb = RBVector2(1.f,0.f);
	rt = RBVector2(1.f,1.f);
	y_direction.x = 0.f;
	y_direction.y = 1.f;
}

void WIPFrameBox::set_quickly( float x1,float y1, float x2,float y2, float x3,float y3, float x4,float y4 ,	float y_x,float y_y)
{
	lt.x = x1;
	lt.y = y1;
	lb.x = x2;
	lb.y = y2;
	rb.x = x3;
	rb.y = y3;
	rt.x = x4;
	rt.y = y4;
	y_direction.x = y_x;
	y_direction.y = y_y;
}