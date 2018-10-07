#include "Sound.h"
#include "AudioManager.h"



const int StudioSound::MAX_STR_LEN = 128;


StudioSound::StudioSound(AudioStudioManager& manager, StudioSound::DescriptionDataRef desc, const std::string& name) :
_manager(manager), _is_playing(false), _name(name),desc_ref(desc),
_started_call_back("started_call_back"),_stopped_call_back("stopped_call_back"),
_restarted_call_back("restarted_call_back"){
  FMOD_CHECK(desc_ref->desc_ref->createInstance(&_inst));
  if (_inst&&_inst->isValid()){
    SetCallback(StudioSound::EventCallback, 
      FMOD_STUDIO_EVENT_CALLBACK_STARTED | 
      FMOD_STUDIO_EVENT_CALLBACK_RESTARTED | 
      FMOD_STUDIO_EVENT_CALLBACK_STOPPED |
      FMOD_STUDIO_EVENT_CALLBACK_DESTROYED);
    //callback need a StudioSound pointer
    SetStudioSound(this);
  }
}

FMOD_RESULT F_CALLBACK StudioSound::EventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters){
	return FMOD_OK;
std::string c;
switch (type)
{
  
case FMOD_STUDIO_EVENT_CALLBACK_STARTED:
  c = "FMOD_STUDIO_EVENT_CALLBACK_STARTED";
  break;
case FMOD_STUDIO_EVENT_CALLBACK_RESTARTED:
  c = "FMOD_STUDIO_EVENT_CALLBACK_RESTARTED";
  break;
case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
  c = "FMOD_STUDIO_EVENT_CALLBACK_STOPPED";
  break;
case FMOD_STUDIO_EVENT_CALLBACK_DESTROYED:
  c = "FMOD_STUDIO_EVENT_CALLBACK_DESTROYED";
  break;
  
}
//LOG_NOTE("push %d->%s\n", event, c.c_str());
  auto obj = new AudioStudioManager::CallbackEventData(type,event,parameters);
  if (!AudioStudioManager::instance()->_event_queue.write(obj))
	  LOG_ERROR("Event queue full!\n");
  return FMOD_OK;
}

StudioSound::~StudioSound(){
  LOG_INFO("[c2]SutdioSound %s:%d Relasesed.Event %d\n",_name.c_str(),this,_inst);
  FMOD_CHECK( _inst->release());
  //c2_log("[c2]Event %d Relasesed.\n", _inst);
  _inst = nullptr;
}

FMOD_STUDIO_PARAMETER_DESCRIPTION StudioSound::GetParamDesc(const std::string& param_name)
{
  FMOD::Studio::ParameterInstance* pi = nullptr;
  FMOD_CHECK(_inst->getParameter(param_name.c_str(), &pi));
  FMOD_STUDIO_PARAMETER_DESCRIPTION  ret = { 0 };
  FMOD_CHECK( pi->getDescription(&ret));
  return ret;
}

float StudioSound::GetParam(const std::string& param_name)
{
  FMOD::Studio::ParameterInstance* pi = nullptr;
  FMOD_CHECK( _inst->getParameter(param_name.c_str(), &pi));
  if (!pi) return -1.f;
  float ret = 0;
  FMOD_CHECK( pi->getValue(&ret));
  return ret;
}

void StudioSound::SetParam(const std::string& param_name,float val)
{
  FMOD::Studio::ParameterInstance* pi = nullptr;
  FMOD_CHECK( _inst->getParameter(param_name.c_str(), &pi));
  if (!pi) return;
  FMOD_CHECK( pi->setValue(val));
}

RBVector3 StudioSound::Get3DPosition()
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = {0};
    FMOD_CHECK( _inst->get3DAttributes(&atb));
    RBVector3 ret;
    ret.x = atb.position.x;
    ret.y = atb.position.y;
    ret.z = atb.position.z;
    return  ret;
  }
  return RBVector3(0, 0, 0);
}

void StudioSound::Set3DPosition(const RBVector3& v)
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    atb.position.x = v.x;
    atb.position.y = v.y;
    atb.position.z = v.z;
    FMOD_CHECK(_inst->set3DAttributes(&atb));
  }
}

RBVector3 StudioSound::Get3DVelocity()
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    RBVector3 ret;
    ret.x = atb.velocity.x;
    ret.y = atb.velocity.y;
    ret.z = atb.velocity.z;
    return  ret;
  }
  return RBVector3(0, 0, 0);
}

void StudioSound::Set3DVelocity(const RBVector3& val)
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    atb.velocity.x = val.x;
    atb.velocity.y = val.y;
    atb.velocity.z = val.z;
    FMOD_CHECK(_inst->set3DAttributes(&atb));
  }
}

RBVector3 StudioSound::Get3DUp()
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    RBVector3 ret;
    ret.x = atb.up.x;
    ret.y = atb.up.y;
    ret.z = atb.up.z;
    return  ret;
  }
  return RBVector3(0, 0, 0);
}

void StudioSound::Set3DUp(const RBVector3& v)
{
  if (_inst){
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    atb.up.x = v.x;
    atb.up.y = v.y;
    atb.up.z = v.z;
    FMOD_CHECK(_inst->set3DAttributes(&atb));
  }
}

RBVector3 StudioSound::Get3DForward()
{
  if (_inst)
  {
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    RBVector3 ret;
    ret.x = atb.forward.x;
    ret.y = atb.forward.y;
    ret.z = atb.forward.z;
    return  ret;
  }
  return RBVector3(0, 0, 0);
}

void StudioSound::Set3DForward(const RBVector3& v)
{
  if (_inst)
  {
    FMOD_3D_ATTRIBUTES atb = { 0 };
    FMOD_CHECK(_inst->get3DAttributes(&atb));
    atb.forward.x = v.x;
    atb.forward.y = v.y;
    atb.forward.z = v.z;
    FMOD_CHECK(_inst->set3DAttributes(&atb));
  }
}

FMOD::Studio::EventDescription* StudioSound::GetEventDesc()
{
  return nullptr;
}

bool StudioSound::IsPause()
{
  if (_inst){
    bool ret = false;
    FMOD_CHECK( _inst->getPaused(&ret));
    return ret;
  }
  return false;
}

void StudioSound::Pause()
{
  if (_inst){
    FMOD_CHECK( _inst->setPaused(true));
  }
}


void StudioSound::Resume()
{
  if (_inst){
    FMOD_CHECK( _inst->setPaused(false));
  }
}


float StudioSound::GetPitch()
{
  if (_inst){
    float ret = 0.f;
    FMOD_CHECK( _inst->getPitch(&ret));
    return ret;
  }
  return -1.f;
}

void StudioSound::SetPitch(float val)
{
  if (_inst){
    FMOD_CHECK(_inst->setPitch(val));
  }
}

int StudioSound::GetTimelinePosition()
{

  if (_inst){
    int ret = 0;
    FMOD_CHECK(_inst->getTimelinePosition(&ret));
    return ret;
  }
  return -1;
}

void StudioSound::SetTimelinePosition(int position)
{

  if (_inst){
    FMOD_CHECK(_inst->setTimelinePosition(position));
  }
}

float StudioSound::GetVolume()
{

  if (_inst){
    float v = 0.f;
    FMOD_CHECK(_inst->getVolume(&v));
    return v;
  }
  return -1.f;
}

void StudioSound::SetVolume(float val)
{

  if (_inst){
    FMOD_CHECK(_inst->setVolume(val) );
  }
}

void StudioSound::Start()
{

  if (_inst){
    _is_playing = true;
    FMOD_CHECK(_inst->start());
  }
}

void StudioSound::Stop(FMOD_STUDIO_STOP_MODE mode)
{

  if (_inst){
    _is_playing = false;
    /*
    FMOD_STUDIO_STOP_ALLOWFADEOUT,
    FMOD_STUDIO_STOP_IMMEDIATE,

    FMOD_STUDIO_STOP_ALLOWFADEOUT
    Allows AHDSR modulators to complete their release, and DSP effect tails to play out.
    */
    FMOD_CHECK(_inst->stop(mode));
  }
}

void StudioSound::SetCallback(FMOD_STUDIO_EVENT_CALLBACK callback,int mask)
{
  if (_inst){
    FMOD_CHECK( _inst->setCallback(callback, mask));
  }
}

void StudioSound::SetCallback(const std::string& callback_name, int callback_type)
{
  switch (callback_type)
  {
  case 1:
    _started_call_back = callback_name;
    break;
  case 2:
    _restarted_call_back = callback_name;
    break;
  case 3:
    _stopped_call_back = callback_name;
    break;
  }
}

float StudioSound::GetChannelPriority()
{
  if (_inst){
    float ret = 0.f; 
    FMOD_CHECK(_inst->getProperty(FMOD_STUDIO_EVENT_PROPERTY_CHANNELPRIORITY, &ret)); 
    return ret; 
  } 
  return -1.f; 
}

float StudioSound::GetScheduleDelay()
{
  if (_inst){
    float ret = 0.f; 
    FMOD_CHECK(_inst->getProperty(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY, &ret)); 
    return ret; 
  } 
  return -1.f; 
}

float StudioSound::GetScheduleLookahead()
{
  if (_inst){
    float ret = 0.f;
    FMOD_CHECK(_inst->getProperty(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_LOOKAHEAD, &ret));
    return ret; 
  } 
  return -1.f; 
}

float StudioSound::GetMinDist()
{
  if (_inst){ 
    float ret = 0.f; 
    FMOD_CHECK(_inst->getProperty(FMOD_STUDIO_EVENT_PROPERTY_MINIMUM_DISTANCE, &ret)); 
    return ret; 
  } return -1.f; 
}

float StudioSound::GetMaxDist()
{ 
  if (_inst){ 
    float ret = 0.f; 
    FMOD_CHECK(_inst->getProperty(FMOD_STUDIO_EVENT_PROPERTY_MAXIMUM_DISTANCE, &ret)); 
    return ret; 
  } 
  return -1.f; 
}

void StudioSound::SetChannelPriority(float val)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setProperty(FMOD_STUDIO_EVENT_PROPERTY_CHANNELPRIORITY, val)); 
  } 
}

void StudioSound::SetScheduleDelay(float val)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setProperty(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY, val)); 
  } 
}

void StudioSound::SetScheduleLookahead(float val)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setProperty(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_LOOKAHEAD, val)); 
  } 
}

void StudioSound::SetMinDist(float val)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setProperty(FMOD_STUDIO_EVENT_PROPERTY_MINIMUM_DISTANCE, val)); 
  } 
}

void StudioSound::SetMaxDist(float val)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setProperty(FMOD_STUDIO_EVENT_PROPERTY_MAXIMUM_DISTANCE, val)); 
  } 
}

void StudioSound::SetStudioSound(StudioSound* userdata)
{ 
  if (_inst){ 
    FMOD_CHECK(_inst->setUserData((void*)userdata)); 
  } 
}

StudioSound* StudioSound::GetStudioSound()
{ 
  if (_inst){ 
    void* ret = nullptr; 
    FMOD_CHECK(_inst->getUserData(&ret)); 
    return (StudioSound*)ret; 
  } 
  return nullptr; 
}

int StudioSound::GetParamCount()
{
  int ret = -1;
  if (!_inst)
    return ret;
  FMOD_CHECK(_inst->getParameterCount(&ret));
  return ret;
}

void StudioSound::PrintAllParam()
{
  int count = GetParamCount();
  FMOD_STUDIO_PARAMETER_DESCRIPTION desc;
  for (int i = 0; i < count;++i){
    FMOD_CHECK(desc_ref->desc_ref->getParameterByIndex(i, &desc));
    LOG_INFO("%d : %s\n",i,desc.name);
  }
  return;
}

bool StudioSound::IsPlaying(){
  return _inst&&_inst->isValid() ? true : false;
}

int StudioSound::GetAllParams(std::vector<std::string>& outv)
{
  int count = GetParamCount();
  FMOD_STUDIO_PARAMETER_DESCRIPTION desc;
  for (int i = 0; i < count; ++i){
    FMOD_CHECK(desc_ref->desc_ref->getParameterByIndex(i, &desc));
    outv.push_back(desc.name);
  }
  return count;
}

bool StudioSound::IsValid(){
  return _inst&&_inst->isValid() ? true : false;
}

void StudioSound::LoadSample(){

}

void StudioSound::UnLoadSamlpe(){

}

bool StudioSound::IsVirtual(){
  bool ret = false;
  if (_inst)
    FMOD_CHECK(_inst->isVirtual(&ret));
  return ret;
}

void StudioSound::DeleteDelay()
{
  FMOD_CHECK( _inst->release());
  LOG_INFO("Event %d Release!\n", _inst);
  _manager._delete_delay.insert(map<FMOD::Studio::EventInstance*, StudioSound*>::value_type(_inst, this));
}


//not use
Bus::Bus(AudioStudioManager& manager,const std::string& name) :
_manager(manager), _name(name)
{
  _bus = nullptr;
  _manager._studio_system->getBus(name.c_str(), &_bus);
}

Bus::~Bus(){
  
}

float Bus::GetFaderLevel()
{
  if (!_bus) return -1.f;
  float ret = -1.f;
  //_bus->getFaderLevel(&ret);
  return ret;
}

bool Bus::GetMute()
{
  if (!_bus) return false;
  bool ret = false;
  _bus->getMute(&ret);
  return ret;
}

bool Bus::GetPaused()
{
  if (!_bus) return false;
  bool ret = false;
  _bus->getPaused(&ret);
  return ret;
}

void Bus::SetFaderLevel(float val)
{
  if (!_bus) return;
  //_bus->setFaderLevel(val);
}

void Bus::SetMute(bool val)
{
  if (!_bus) return;
  _bus->setMute(val);
}

void Bus::SetPaused(bool val)
{
  if (!_bus) return;
  _bus->setPaused(val);
}

void Bus::StopAllEvents(FMOD_STUDIO_STOP_MODE mode)
{
  if (!_bus) return;
  _bus->stopAllEvents(mode);
}



