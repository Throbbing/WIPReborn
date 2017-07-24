#pragma once
#include "RBMath/Inc/Vector2.h"

class WIPFrameBox
{
public:
	 RBVector2 lt,lb,rb,rt;
	 RBVector2 y_direction;
	 	void set_quickly
		(
		float lt_x1,float lt_y1,
		float lb_x2,float lb_y2,
		float rb_x3,float rb_y3,
		float rt_x4,float rt_y4,
		float y_x,float y_y
		);
		WIPFrameBox();
};
