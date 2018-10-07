#include "State.h"

State::State(StateController * sc):_controller(sc)
{
	
}

StateController::StateController():_current_state(nullptr)
{

}

void StateController::set_state(State * s)
{
	if (_current_state)
		_current_state->end();
	_current_state = s;
	_current_state->begin();
}

State * StateController::get_current_state() const
{
	return _current_state;
}

void StateController::update(float dt)
{
	if(_current_state)
		_current_state->update(dt);
}
