#include "Profiler.h"
#include "Logger.h"
//#include "SoftR/Rasterizer.h"

void Profiler::out_put_after_time(int times)
{

	if (times < 0)
		times = _times;
	_out_cur_times += 1;
	if (_out_cur_times >= times)
	{
		float it = 1.f / times;
		std::string str;
		for (GartherMap::iterator i = _gather_element_ex.begin(); i != _gather_element_ex.end(); ++i)
		{
			std::string a = i->first;
			char num[64];
			sprintf(num, ": %f  ", i->second*it);
			a += num;
			i->second = 0.f;
			str += a;
		}
		str += "\n";
		//char a[128];
		//sprintf(a, "TriSet:%.0f,ScanLine:%.0f,PS+OM:%.0f,PS:%.2f,OM:%.0f,Scanline_lerp:%.0f,TSP:%.0f\n", it*_gather_elements[0],it*_gather_elements[1],it*_gather_elements[2],it*_gather_elements[3],it*_gather_elements[4],it*_gather_elements[5],it*_gather_elements[6]);
		LOG_INFO(str.c_str());
		/*
		for (int i = 0; i < max_elements;++i )
		{
		_gather_elements[i] = 0.f;
		}
		*/
		_out_cur_times = 0;
	}
}
