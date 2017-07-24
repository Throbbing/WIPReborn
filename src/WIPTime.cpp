#include "WIPTime.h"
#include <algorithm>

#ifdef _WIN32
#include "windows.h"

TimeSource::TimeSource()
{
	LARGE_INTEGER __freq;
	QueryPerformanceFrequency(&__freq);
	//_secs_per_tick = 1000.0/__freq.QuadPart;
	_secs_per_tick = 1.0/__freq.QuadPart;
}


double TimeSource::get_time() const
{
	LARGE_INTEGER __time;
	QueryPerformanceCounter(&__time);
	//printf("%f\n",__time.QuadPart * _secs_per_tick);
	return __time.QuadPart * _secs_per_tick;
}
#else
#include <ctime>
TimeSource::TimeSource()
{

}

double TimeSource::get_time() const
{
    clock_t t = clock();//ms
	return t/1000.0;
}
#endif

RBClock* RBClock::_clock_instance = 0;

RBClock::RBClock(const TimeSource* a_timesource)
{
	_time_source = NULL;
	_current_time = 0;
	_frame_time = 0;
	_frame_time = 0;
	_source_last_value = 0;
	_source_start_value = 0;
	reset_time_source(a_timesource);
	set_filtering(1);
	_frame_time = get_predicted_frame_time();

}

RBClock::~RBClock()
{
	//delete _time_source;
	//delete _clock_instance;
}

RBClock* RBClock::Instance()
{
	
	return _clock_instance;

}

INI_RESULT RBClock::init(TimeSource* a_timesource)
{
	if(!_clock_instance)
		_clock_instance = new RBClock(a_timesource);
	if(_clock_instance)
	{

		return INI_SUCCESS;
	}
	else
		return INI_FAIL;

}

void RBClock::reset_time_source(const TimeSource* a_timesource)
{
	delete _time_source;
	_time_source = a_timesource;
	if(_time_source!=NULL)
	{
		_source_start_value = _time_source->get_time();
		_source_last_value =_source_start_value;
	}
}

void RBClock::update()
{
	f64 exact_last_frame_time = get_exact_last_frame_time();
	add_to_frame_history(exact_last_frame_time);

	//���ܲ���
	_frame_time = get_predicted_frame_time();
	_current_time += _frame_time;

	_frame_number++;

	std::vector<ITimeObserver*>::iterator it;
	for(it = _observers.begin() ; it != _observers.end() ; ++it)
		(*it)->notify();
}

f64 RBClock::get_exact_last_frame_time()
{
	f64 source_time;
	if(_time_source==NULL)
		source_time = 0;
	else
	{
		source_time = _time_source->get_time();
	}

	f64 frame_time = source_time - _source_last_value;
	if(frame_time>0.200)
		frame_time = _frame_time_history.back();
	_source_last_value = source_time;
	return frame_time;
}

void RBClock::add_to_frame_history(f64 exact_frame_time)
{
	_frame_time_history.push_back(exact_frame_time);
	if(_frame_time_history.size()>(u32)_frame_filtering_window)
		_frame_time_history.pop_front();
}

//���ܲ���
f64 RBClock::get_predicted_frame_time() const
{
	f64 total_frame_time = 0;
	std::deque<f64>::const_iterator it;
	for(it = _frame_time_history.begin();it!=_frame_time_history.end();++it)
		total_frame_time += *it;
	return total_frame_time/_frame_time_history.size();
}

void RBClock::add_observer(ITimeObserver* observer)
{
	if(observer!=NULL)
		_observers.push_back(observer);
}

void RBClock::remove_observer(ITimeObserver* observer)
{
	_observers.erase(std::remove(_observers.begin(),_observers.end(),observer),_observers.end());
}


void RBClock::set_filtering(i32 frame_window,f64 frame_default /* = 0.030 */)
{
	_frame_filtering_window = frame_window > 1?frame_window:1;
	_frame_default_time = frame_default;
	_frame_time_history.clear();
	_frame_time_history.push_back(_frame_default_time);
}

RBTimerBase::RBTimerBase(RBClock& clock):_clock(clock)
{
	_current_time = _frame_time = 0;
	_bpaused = false;
	_scale = 1;
	_clock.add_observer(this);
}

RBTimerBase::~RBTimerBase()
{
	_clock.remove_observer(this);
}

void RBTimerBase::notify()
{
	if(!_bpaused)
	{
		_frame_time = _clock.get_frame_time()*_scale;
		_current_time += _frame_time;
	}
}

void RBTimerBase::pause(bool bOn)
{
	_bpaused = bOn;
}

void RBTimerBase::set_scale(f32 fScale)
{
	_scale = fScale;
}

f32 RBTimerBase::get_scale() const
{
	return _scale;
}

bool RBTimerBase::is_paused() const
{
	return _bpaused;
}