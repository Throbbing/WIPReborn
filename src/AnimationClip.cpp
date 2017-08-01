#include "AnimationClip.h"
#include "FrameBox.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "IniHelper.h"
#include "Logger.h"

//tansform coord to lb
bool read_animation_atlas_file(const char* file_name,int& total_frame,float **out_uvs,std::string& texture_name)
{
	bool ret = true;
	auto text_handler = g_res_manager->load_resource(file_name,WIPResourceType::TEXT);
	const char *text = (((string *)text_handler->ptr)->c_str());
	WIPIniHelper::reset_ini_from_text(text,text_handler->size);
	//WIPIniHelper::reset_ini_file(file_name);
	if(!WIPIniHelper::get_int("head","total_frame",total_frame))
	{
		g_logger->debug_print(WIP_NOTE,"load clip total_frame failed!");
		return false;
	}
	if(!WIPIniHelper::get_string("head","texture",texture_name))
	{
		g_logger->debug_print(WIP_INFO,"read clip texture infomation failed!");
		return false;
	}

	*out_uvs = new float[total_frame*8];

	
	for(int i=0;i<total_frame;++i)
	{
		char s[32];
		sprintf(s,"%d",i+1);
		WIPIniHelper::get_float(s,"lt_x",(*out_uvs)[i*8+0]);
		WIPIniHelper::get_float(s,"lt_y",(*out_uvs)[i*8+1]);
		WIPIniHelper::get_float(s,"lb_x",(*out_uvs)[i*8+2]);
		WIPIniHelper::get_float(s,"lb_y",(*out_uvs)[i*8+3]);
		WIPIniHelper::get_float(s,"rb_x",(*out_uvs)[i*8+4]);
		WIPIniHelper::get_float(s,"rb_y",(*out_uvs)[i*8+5]);
		WIPIniHelper::get_float(s,"rt_x",(*out_uvs)[i*8+6]);
		WIPIniHelper::get_float(s,"rt_y",(*out_uvs)[i*8+7]);
		(*out_uvs)[i * 8 + 1] = 1 - (*out_uvs)[i * 8 + 1];
		(*out_uvs)[i * 8 + 3] = 1 - (*out_uvs)[i * 8 + 3];
		(*out_uvs)[i * 8 + 5] = 1 - (*out_uvs)[i * 8 + 5];
		(*out_uvs)[i * 8 + 7] = 1 - (*out_uvs)[i * 8 + 7];
		f32 temp = (*out_uvs)[i * 8 + 1];
		(*out_uvs)[i * 8 + 1] = (*out_uvs)[i * 8 + 3];
		(*out_uvs)[i * 8 + 3] = temp;
		temp = (*out_uvs)[i * 8 + 5];
		(*out_uvs)[i * 8 + 5] = (*out_uvs)[i * 8 + 7];
		(*out_uvs)[i * 8 + 7] = temp;

	}
	
	WIPIniHelper::close();
	return ret;
}

WIPAnimationClip::WIPAnimationClip(const char* name,bool loop)
{
	bloop = loop;
	bplaying = false;
	this->name = name;
	_total_frame = 0;
	_uvs = 0;
	_speed = 1.f;
	_cur_dt = 0.f;
	_cur_frame = 1;
	_will_stop = false;
}


WIPAnimationClip* WIPAnimationClip::create_with_atlas(const char* name,const char* atlas_file)
{
	std::string texture_name;
	WIPAnimationClip* ret= new WIPAnimationClip(name,false);
	if(!read_animation_atlas_file(atlas_file,ret->_total_frame,&ret->_uvs,texture_name))
	{
		delete ret;
		return 0;
	}
	ret->texture_type = ATLAS;
	return ret;
}

WIPAnimationClip::~WIPAnimationClip()
{
	delete[] _uvs;
}