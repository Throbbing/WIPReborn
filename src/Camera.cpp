#if 1
#include "Camera.h"
#include "Sprite.h"
#include "Logger.h"
#include "RBMath/Inc/RBMath.h"
#include "Scene.h"
#include <algorithm>
#include "Render.h"



WIPCamera::WIPCamera(f32 w, f32 h, f32 sw, f32 sh)
{
	world_x = world_y = 0.f;
	world_h = h;
	world_w = w;
	_active = true;
	_zoom = 1.f;
	viewport = g_rhi->RHICreateViewPort(0, 0, sw, sh);
}

WIPCamera::~WIPCamera()
{
}

void WIPCamera::reset_ratio(f32 ratio)
{
	f32 new_height = world_w * ratio;
	i32 height = static_cast<i32>(new_height);
	world_h = height;
}

RBVector2 WIPCamera::camera_to_world(const RBVector2& camera_pos)
{
	return RBVector2(world_x+camera_pos.x,world_y+camera_pos.y);
}

RBVector2 WIPCamera::world_to_camera(const RBVector2& world_pos)
{
	return RBVector2(world_pos.x-world_x,world_pos.y-world_y);
}

void WIPCamera::set_background_color(const WIPColorf& color)
{

}

void WIPCamera::move(f32 dx,f32 dy)
{
	world_x += dx;
	world_y += dy;
}

void WIPCamera::move_to(f32 x,f32 y)
{
	world_x = x;
	world_y = y;
}


void WIPCamera::set_active(bool val)
{
	_active = val;
}

bool WIPCamera::get_active()
{
	return _active;
}

#endif