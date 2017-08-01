#pragma once
#include "RBMath.h"

class WIPBaseEditor
{
public:
	void on_gui(f32 dt);
};

class WIPFrameAnimationEditor : public WIPBaseEditor
{
public:
	void on_gui(f32 dt);

};

class WIPEditorMode
{
public:
	void on_gui();
	void on_frame_animation_gui(f32 dt);

};