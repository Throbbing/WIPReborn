#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include <cstddef>
#include <vector>
#include <deque>
#include "RefCount.h"

class TimeSourceBase : public FRefCountedObject
{
public:
    virtual ~TimeSourceBase() {};
    virtual double get_time() const = 0;
};

#ifdef _WIN32
class TimeSource:public TimeSourceBase
{
public:
	TimeSource();
	virtual double get_time() const;
private:
	double _secs_per_tick;
};
#else
class TimeSource:public TimeSourceBase
{
public:
	TimeSource();
	virtual double get_time() const;
private:
	//double _secs_per_tick;
};
#endif

class ITimeObserver : public FRefCountedObject
{
public:
	virtual ~ITimeObserver(){};
	virtual void notify() = 0;
};

class RBClock : public FRefCountedObject
{
public:
	~RBClock();

	static RBClock* Instance();

	static INI_RESULT init(TimeSource* a_timesource);

	void reset_time_source(const TimeSource* a_timesource);

	void update();

	f64 get_time() const
	{
		return _current_time;
	}

	f64 get_frame_time() const
	{
		return _frame_time;
	}

	i32 get_frame_number() const
	{
		return _frame_number;
	}

	f32 get_frame_rate() const
	{
		return 1.0f/(f32)_frame_time;
	}


	void add_observer(ITimeObserver* observer);
	void remove_observer(ITimeObserver* observer);

	void set_filtering(i32 frame_window,f64 frame_default = 0.030);

	//time_t get_system_time();
protected:
	RBClock();
	RBClock(const TimeSource* a_timesource);
private:
	f64 get_exact_last_frame_time();
	void add_to_frame_history(f64 exact_frame_time);
	//Ԥ��ʱ��
	f64 get_predicted_frame_time() const;


	static RBClock* _clock_instance;
	const TimeSource* _time_source;
	f64 _current_time;
	f64 _frame_time;
	i32 _frame_number;

	f64 _source_start_value;
	f64 _source_last_value;

	i32 _frame_filtering_window;
	f64 _frame_default_time;
	std::deque<f64> _frame_time_history;

	std::vector<ITimeObserver*> _observers;
};





class RBTimerBase : public ITimeObserver
{
public:
	RBTimerBase(RBClock& clock);
	~RBTimerBase();

	//get current time
	f64 get_time() const
	{
		return _current_time;
	}

	f64 get_time_ms() const
	{
		return _current_time*1000.0;
	}

	//get predicted time
	f64 get_frame_time() const
	{
		return _frame_time;
	}

	void pause (bool bOn);
    void set_scale (f32 fScale);   

	bool is_paused () const;
    f32 get_scale () const;
private:
	virtual void notify();

	RBClock& _clock;

	f64 _current_time;
	f64 _frame_time;

	bool _bpaused;
	f32 _scale;
};