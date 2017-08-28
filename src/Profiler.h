#pragma once

#include "RBTimer.h"
#include <map>
#include <string>


class Profiler
{
private:
	typedef std::map<std::string,float> GartherMap;
public:

	static const int max_elements = 10;
	enum ProfTypes
	{
		e_ia,
		e_vs,
		e_gs,
		e_nfc,
		e_bc,
		e_clip,
		e_tsp,
	};
	void init()
	{
		_timer.init();
		_out_cur_times = 0.f;
		_cur_time = 0.f;
		for (int i = 0; i < max_elements; ++i)
		{
			_gather_elements[i] = 0.f;
		}
	}
	void set_begin()
	{
		_timer.begin();
	}
	void set_end(int index)
	{
		_gather_elements[index] += _timer.end();
	}
	void set_end(std::string name)
	{
		double tt = _timer.end();
		GartherMap::iterator it = _gather_element_ex.find(name);
		if(it==_gather_element_ex.end())
		{
			_gather_element_ex.insert(GartherMap::value_type(name,tt));
		}
		else
		{
			_gather_element_ex[name] += tt;
		}

		if (name == "TS")
			_last_ts_time = tt;

	}
	void gather_element(int index, float time)
	{
		_gather_elements[index] += time;
	}
	float get_element(int index)
	{
		return _gather_elements[index];
	}

	float get_time()
	{
		return _last_ts_time;
	}
	void clear()
	{
		for (int i = 0; i < max_elements; ++i)
		{
			_gather_elements[i] = 0.f;
		}
		_gather_element_ex.clear();
	}

	//how many times to output
	void set_output_inter(int times)
	{
		_times = times;
	}

	void out_put_after_time(int times);
private:
	int _times;
	float _cur_time;
	float _out_cur_times;
	float _gather_elements[max_elements];
	rb::rb_timer_t _timer;

	GartherMap _gather_element_ex;

	float _last_ts_time;
};