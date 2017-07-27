#if 1
#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include "RBMath/Inc/Vector2.h"

class WIPRenderTexture;
class WIPLayer;
class WIPRenderBatch;
class WIPShader;
class WIPColorf;
class WIPMesh;
class WIPViewPort;
class WIPScene;
class WIPSprite;

#define ORIGIN_EFFECT	1
#define AFTER_EFFECT		2

/*
     |1
	 |
-1___0_____1
     |
	 |
	 |-1
*/
class WIPCamera
{
public:
	WIPCamera(f32 w,f32 h,f32 pw,f32 ph,int iww,int iwh);
	~WIPCamera();
	//left button
	f32 world_x,world_y;
	f32 world_w,world_h;
	//left top
	WIPViewPort* viewport;

	
	void copy_from(WIPCamera* other);
	void clear();
	//keep width fixed
	void reset_ratio(f32 ratio);
	RBVector2 camera_to_world(const RBVector2& camera_pos);
	RBVector2 world_to_camera(const RBVector2& world_pos);
	RBVector2 screen_to_world(const RBVector2I& screen_pos);
	void set_background_color(const WIPColorf& color);
	void move(f32 x,f32 y);
	void move_to(f32 x,f32 y);
	inline float get_zoom(){ return _zoom; }
	inline void zoomout(float dz){ _zoom += dz; }
	inline void zoomin(float dz){ _zoom -= dz; _zoom = RBMath::clamp(_zoom, 0.001f, _zoom); }
	void set_active(bool val);
	bool get_active();


public:

	bool _active;
	float _zoom;
	int window_w;
	int window_h;

};
#endif