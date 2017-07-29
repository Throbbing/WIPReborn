#include "AudioManager.h"
#include "Sound.h"
#include "AudioFmodSystem.h"
#include "ResourceManager.h"

const int AudioStudioManager::MAX_CHANNEL_COUNT = 16;
const int AudioStudioManager::MAX_BUS_COUNT = 64;
const int AudioStudioManager::MAX_INSTANCE_COUNT = 128;
const int AudioStudioManager::MAX_QUEUE_SIZE = 64;

AudioStudioManager* AudioStudioManager::instance()
{
	static AudioStudioManager* _instance;
	if (!_instance)
		_instance = new AudioStudioManager();
	return _instance;
}

AudioStudioManager::AudioStudioManager():
_event_queue(MAX_QUEUE_SIZE)
{
  _studio_system = nullptr;
  FMOD_CHECK( FMOD::Studio::System::create(&_studio_system));
  _lowlevel_system = nullptr;
  FMOD_CHECK(_studio_system->getLowLevelSystem(&_lowlevel_system));
  FMOD_CHECK(_lowlevel_system->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));
  FMOD_INITFLAGS flags;
  flags = FMOD_STUDIO_INIT_LIVEUPDATE|FMOD_STUDIO_INIT_NORMAL;
  FMOD_CHECK(_studio_system->initialize(MAX_CHANNEL_COUNT, flags, FMOD_INIT_3D_RIGHTHANDED, nullptr));
  
  auto ret = _studio_system->flushCommands();
  if (ret == FMOD_ERR_NET_SOCKET_ERROR) 
  {
    flags &= ~FMOD_STUDIO_INIT_LIVEUPDATE;
    FMOD_CHECK(_studio_system->unloadAll());
    FMOD_CHECK(_studio_system->release());
    FMOD_CHECK(FMOD::Studio::System::create(&_studio_system));
    FMOD_CHECK(_studio_system->getLowLevelSystem(&_lowlevel_system));
    FMOD_CHECK(_lowlevel_system->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));
    FMOD_CHECK(_studio_system->initialize(MAX_CHANNEL_COUNT, flags, FMOD_INIT_3D_RIGHTHANDED, nullptr));
  } 
  else if (ret != FMOD_OK) 
  {
    LOG_INFO("[C2] Failed to initialize FMOD Studio: %s.\n", FMODErrorMessage[ret]);
    abort();
  }
  

  SetListenerPosition(0, RBVector3(0, 0, 0));
  SetListenerVelocity(0, RBVector3(0, 0, 0));
  SetListenerUp(0, RBVector3(0, 1.f, 0));
  SetListenerForward(0, RBVector3(0, 0, -1.f));

  _last_error = FMOD_OK;
}

AudioStudioManager::~AudioStudioManager()
{
  StopAll();
  ClearAllSound();
  UnloadAllBanks();
  _studio_system->release();
  _lowlevel_system = nullptr;
}

void AudioStudioManager::Update() 
{
  _studio_system->update();

  CallbackEventDataRef ce = nullptr;
  while (_event_queue.read(ce))
  {
    auto type = ce->type;
    auto event = ce->event;
    auto parameters = ce->parameters;
    FMOD::Studio::EventDescription* desc = nullptr;
    FMOD::Studio::EventInstance* event_ = (FMOD::Studio::EventInstance*)event;
    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED)
	{
      delete _delete_delay[event_];
      _delete_delay.erase(event_);
      return;
    }

    if (!event_ ||!event_->isValid())
	{
      LOG_WARN("Invalid Event %d!\n",event_);
      std::string s;
      switch (type)
      {
      case FMOD_STUDIO_EVENT_CALLBACK_STARTED:
        s = "FMOD_STUDIO_EVENT_CALLBACK_STARTED";
        break;
      case FMOD_STUDIO_EVENT_CALLBACK_RESTARTED:
        s = "FMOD_STUDIO_EVENT_CALLBACK_RESTARTED";
        break;
      case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
        s = "FMOD_STUDIO_EVENT_CALLBACK_STOPPED";
        break;
      }
      LOG_NOTE("Event %d type:%s\n", event_,s.c_str());
      //it proves that you don't want to care the instance if you release it.
      //so its callback disappears.
      
        return;
    }
    FMOD_CHECK(event_->getDescription(&desc));
    char str[128];
    int s = 0;
    StudioSound* sound_ = nullptr;
    FMOD_CHECK(event_->getUserData((void **)&sound_));
    StudioSound* p_ = sound_;
    if (!p_ || !p_->_inst || !p_->_inst->isValid())
	{
      LOG_WARN("Invalid UserData sound:%d,sound inst:%d,inst valid:%s!\n", p_, p_->_inst, !p_->_inst->isValid()?"true":"false");
      return;
    }
    FMOD_CHECK(desc->getPath(str, 128, &s));
    switch (type){
    case FMOD_STUDIO_EVENT_CALLBACK_STARTED:
      //AudioStudioManager::Instance()->NotifyEx(sound_->_started_call_back.GetCstd::string(), [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
      break;
    case FMOD_STUDIO_EVENT_CALLBACK_RESTARTED:
      //AudioStudioManager::Instance()->NotifyEx(sound_->_restarted_call_back.GetCstd::string(), [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
      break;
    case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
      //AudioStudioManager::Instance()->NotifyEx(sound_->_stopped_call_back.GetCstd::string(), [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
      break;
    }
  }
}

bool AudioStudioManager::LoadBank(const std::string& filename, bool async){
  bool ret = true;
  do{
    if (!_studio_system){
      LOG_ERROR("[C2]FMOD System did not initialize!\n");
      ret = false;
      break;
    }
	auto res_handler = g_res_manager->load_resource(filename.c_str(), WIPResourceType::EFont);
    FMOD::Studio::Bank* bank = nullptr;
    //if (FMOD_OK != _studio_system->loadBankFile(filename.GetCstd::string(), FMOD_STUDIO_LOAD_BANK_NONBLOCKING, &bank))
     if ( !FMOD_CHECK(_studio_system->loadBankMemory((const char*)res_handler->ptr, res_handler->size, 
       FMOD_STUDIO_LOAD_MEMORY, async ? 
     FMOD_STUDIO_LOAD_BANK_NONBLOCKING :
       FMOD_STUDIO_LOAD_BANK_NORMAL, &bank))){
      LOG_ERROR("[C2]FMOD loadBankMemory failed!\n");
      ret = false;
      break;
    }
    FMOD_CHECK( _studio_system->flushCommands());
     FMOD_STUDIO_LOADING_STATE st;
     FMOD_CHECK( bank->getLoadingState(&st));
     if (FMOD_STUDIO_LOADING_STATE_LOADED == st){
       _bank_vector.push_back(bank);
	   int count;
	   FMOD::Studio::EventDescription* es[10];
	   bank->getEventList(es, 10, &count);
	   char na[10];
	   es[0]->getPath(na, 10, &count);
	   LOG_NOTE("[C2]Bank:%s loaded!\n", filename.c_str());
       int temp = -1;
       FMOD_CHECK(_studio_system->getBankCount(&temp));
	   LOG_INFO("[C2]Current Bank count:%d\n", temp);
     }
  } while (false);
  return ret;
}

void AudioStudioManager::Play(StudioSoundRef sound){
  if (!sound)
  {
	LOG_WARN("[C2]Play a nil sound!\n");
    return;
  }
  sound->Start();
}

StudioSoundRef AudioStudioManager::Play(const std::string& event_name){
  auto temp = CreateSound(event_name);
  if (temp)
    temp->Start();
  return temp;
}

void AudioStudioManager::Stop(StudioSoundRef sound, FMOD_STUDIO_STOP_MODE mode){
  if (!sound){
	  LOG_WARN("[C2]Stop a nil sound!\n");
    return;
  }
  sound->Stop(mode);
}

void AudioStudioManager::Stop(const std::string& event_name, FMOD_STUDIO_STOP_MODE mode /*= FMOD_STUDIO_STOP_IMMEDIATE*/){
  auto it = _event_desc_map.find(event_name);
  if (it==_event_desc_map.end()){
	  LOG_WARN("[c2]Invalid event name:%s", event_name.c_str());
  }
  else{
    it->second->StopAll();
  }
}

void AudioStudioManager::StopAll(FMOD_STUDIO_STOP_MODE mode){
  for (auto i : _event_desc_map){
    i.second->StopAll();
  }
}

StudioSoundRef AudioStudioManager::CreateSound(const std::string& name, const std::string& path){
  return nullptr;
}

StudioSoundRef AudioStudioManager::CreateSound(const std::string& event_name){
  auto desc = GetDescription(event_name);
  if (desc){
	  return new StudioSound(*this, desc, "default");
		//std::mem_fn(&StudioSound::DeleteDelay));
    //auto ret = make_shared<StudioSound>(*this,desc,"default");
  }
  else{
    LOG_WARN("[c2]Invalid event name:%s", event_name.c_str());
    return nullptr;
  }
}


void AudioStudioManager::SetDefaultCallbacks(FMOD::Studio::EventDescription* desc)
{
  //one event could only set one callback using mask to filter some event.
  FMOD_CHECK(desc->setCallback(AudioStudioManager::DefaultEventCallback, 
    FMOD_STUDIO_EVENT_CALLBACK_STARTED | 
    FMOD_STUDIO_EVENT_CALLBACK_RESTARTED | 
    FMOD_STUDIO_EVENT_CALLBACK_STOPPED|
    FMOD_STUDIO_EVENT_CALLBACK_DESTROYED));
}

FMOD_RESULT F_CALLBACK AudioStudioManager::DefaultEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters)
{
  FMOD::Studio::EventDescription* desc = nullptr;
  FMOD::Studio::EventInstance* event_ = (FMOD::Studio::EventInstance*)event;
  FMOD_CHECK( event_->getDescription(&desc));
  char str[128];
  int s = 0;
  StudioSound* sound_;
  FMOD_CHECK( event_->getUserData((void **)&sound_));
  //shared_ptr<StudioSound> p_(sound_);
  StudioSound* p_ = sound_;
  if (!p_ || !p_->_inst || !p_->_inst->isValid()) return FMOD_ERR_INVALID_HANDLE;
  FMOD_CHECK(desc->getPath(str,128,&s));
  switch (type){
  case FMOD_STUDIO_EVENT_CALLBACK_STARTED:
    //c2_log("[C2]Default Callback : started callback ->Event:%s EventInstance:%d\n",str,event);
    //AudioStudioManager::Instance()->NotifyEx("started_call_back", [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
    break;
  case FMOD_STUDIO_EVENT_CALLBACK_RESTARTED:
    //c2_log("[C2]Default Callback : restarted callback ->Event:%s EventInstance:%d\n", str, event);
    //AudioStudioManager::Instance()->NotifyEx("restarted_call_back", [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
    break;
  case FMOD_STUDIO_EVENT_CALLBACK_STOPPED:
    //c2_log("[C2]Default Callback : stopped callback ->Event:%s EventInstance:%d\n", str, event);
    //AudioStudioManager::Instance()->NotifyEx("stopped_call_back", [&](LuaState& L)->int{L.PushScriptObject(p_); return 1; });
    break;
  }
  return FMOD_OK;
}

void AudioStudioManager::StartCommandCapture(const std::string& recordfile)
{
  if (!_studio_system) return;
  FMOD_CHECK(_studio_system->startCommandCapture(recordfile.c_str(), FMOD_STUDIO_COMMANDCAPTURE_NORMAL));
}

void AudioStudioManager::StopCommandCapture()
{
  if (!_studio_system) return;
  FMOD_CHECK(_studio_system->stopCommandCapture());
}

void AudioStudioManager::SetListenerPosition(int index, const RBVector3& p)
{
  if (!_studio_system) return;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index,&attributes));
  attributes.position.x = p.x;
  attributes.position.y = p.y;
  attributes.position.z = p.z;
  FMOD_CHECK(_studio_system->setListenerAttributes(index,&attributes));
}

RBVector3 AudioStudioManager::GetListenerPosition(int index){
  if (!_studio_system) return RBVector3(0,0,0);
  RBVector3 ret;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  ret.x = attributes.position.x;
  ret.y = attributes.position.y;
  ret.z = attributes.position.z;
  return ret;
}

void AudioStudioManager::SetListenerVelocity(int index, const RBVector3& p)
{
  if (!_studio_system) return;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  attributes.velocity.x = p.x;
  attributes.velocity.y = p.y;
  attributes.velocity.z = p.z;
  FMOD_CHECK(_studio_system->setListenerAttributes(index, &attributes));
}

RBVector3 AudioStudioManager::GetListenerVelocity(int index)
{
  if (!_studio_system) return RBVector3(0, 0, 0);
  RBVector3 ret;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  ret.x = attributes.velocity.x;
  ret.y = attributes.velocity.y;
  ret.z = attributes.velocity.z;
  return ret;
}

void AudioStudioManager::SetListenerUp(int index, const RBVector3& p)
{
  if (!_studio_system) return;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  attributes.up.x = p.x;
  attributes.up.y = p.y;
  attributes.up.z = p.z;
  FMOD_CHECK(_studio_system->setListenerAttributes(index, &attributes));
}

RBVector3 AudioStudioManager::GetListenerUp(int index)
{
  if (!_studio_system) return RBVector3(0, 0, 0);
  RBVector3 ret;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  ret.x = attributes.up.x;
  ret.y = attributes.up.y;
  ret.z = attributes.up.z;
  return ret;
}

void AudioStudioManager::SetListenerForward(int index, const RBVector3& p)
{
  if (!_studio_system) return;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  attributes.forward.x = p.x;
  attributes.forward.y = p.y;
  attributes.forward.z = p.z;
  FMOD_CHECK(_studio_system->setListenerAttributes(index, &attributes));
}

RBVector3 AudioStudioManager::GetListenerForward(int index)
{
  if (!_studio_system) return RBVector3(0, 0, 0);
  RBVector3 ret;
  FMOD_3D_ATTRIBUTES attributes = { { 0 } };
  FMOD_CHECK(_studio_system->getListenerAttributes(index, &attributes));
  ret.x = attributes.forward.x;
  ret.y = attributes.forward.y;
  ret.z = attributes.forward.z;
  return ret;
}

void AudioStudioManager::FlushFmodCommands()
{
  if (!_studio_system) return;
  FMOD_CHECK( _studio_system->flushCommands());
}

void AudioStudioManager::LoadAllBuses()
{
  /*
  int bus_count = 0;
  FMOD::Studio::Bus* buses[MAX_BUS_COUNT];
  for (auto i : _bank_vector){
    //i->getBusCount(&bus_count);
    i->getBusList(buses, MAX_BUS_COUNT, &bus_count); 
    for (auto j : buses){
      //char* s[128];
      //j->getPath()
    }
  }
  */
}

float AudioStudioManager::GetFaderLevel(const std::string& name)
{
  auto bus = GetBus(name);
  if (!bus) return -1.f;
  float ret = -1.f;
  //FMOD_CHECK( bus->getFaderLevel(&ret));
  return ret;
}

bool AudioStudioManager::GetMute(const std::string& name)
{
  auto bus = GetBus(name);
  if (!bus) return false;
  bool ret = false;
  FMOD_CHECK( bus->getMute(&ret));
  return ret;
}

bool AudioStudioManager::GetPaused(const std::string& name)
{
  auto bus = GetBus(name);
  if (!bus) return false;
  bool ret = false;
  FMOD_CHECK( bus->getPaused(&ret));
  return ret;
}

void AudioStudioManager::SetFaderLevel(const std::string& name, float val)
{
  auto bus = GetBus(name);
  if (!bus) return;
  //FMOD_CHECK (bus->setFaderLevel(val));
}

void AudioStudioManager::SetMute(const std::string& name, bool val)
{
  auto bus = GetBus(name);
  if (!bus) return ;
  FMOD_CHECK( bus->setMute(val));
}

void AudioStudioManager::SetPaused(const std::string& name, bool val)
{
  auto bus = GetBus(name);
  if (!bus) return ;
  FMOD_CHECK( bus->setPaused(val));
}

void AudioStudioManager::StopAllEvents(const std::string& name, FMOD_STUDIO_STOP_MODE mode)
{
  auto bus = GetBus(name);
  if (!bus) return ;
  FMOD_CHECK( bus->stopAllEvents(mode));
}

void AudioStudioManager::ClearAllSound()
{
  StopAll();
}

void AudioStudioManager::DeleteSound(StudioSoundRef sound)
{
  if (sound){
    (sound)->Stop();
  }
}

void AudioStudioManager::UnloadAllBanks()
{
  for (auto i : _bank_vector){
    i->unload();
  }
}

const char* AudioStudioManager::GetLastError()
{
  return FMODErrorMessage[_last_error];
}

void AudioStudioManager::SetLastError(FMOD_RESULT rescode)
{
  _last_error = rescode;
}


void AudioStudioManager::ReleaseSoundInstance(StudioSoundRef sound)
{
  if (sound&&sound->_inst&&sound->_inst->isValid()){
    sound->Stop();
    FMOD_CHECK( sound->_inst->release());
    sound->desc_ref = nullptr;
  }
}

StudioSound::DescriptionDataRef AudioStudioManager::GetDescription(const std::string& desc_name)
{
  FMOD::Studio::EventDescription* des = nullptr;
  auto it = _event_desc_map.find(desc_name);
  if (it == _event_desc_map.end()){
    FMOD_CHECK( _studio_system->getEvent(desc_name.c_str(),&des));
    if (des&&des->isValid()){
      auto temp = new StudioSound::DescriptionData(des);
      _event_desc_map.insert(FmodEventMap::value_type(desc_name, temp));
      return temp;
    }
    else{
		LOG_WARN("[c2]Invalid event name : %s \n", desc_name.c_str());
      return nullptr;
    }
  }
  else{
    if (!it->second->desc_ref->isValid()){
      //maybe bank has been unloaded.
		LOG_WARN("[c2]Event %s removed.\n", desc_name.c_str());
      _event_desc_map.erase(desc_name);
      return nullptr;
    }
    else{
      return it->second;
    }
  }
}

int AudioStudioManager::GetInstanceCount(const std::string& desc_name){
  int ret = -1;
  auto desc = GetDescription(desc_name);
  if (desc){
    FMOD_CHECK( desc->desc_ref->getInstanceCount(&ret));
  }
  return ret;
}

void StudioSound::DescriptionData::StopAll(FMOD_STUDIO_STOP_MODE mode){
  int c = 0;
  FMOD::Studio::EventInstance* da[AudioStudioManager::MAX_INSTANCE_COUNT];
  FMOD_CHECK(desc_ref->getInstanceList(da, AudioStudioManager::MAX_INSTANCE_COUNT, &c));
  for (int j = 0; j < c; ++j){
    FMOD_CHECK(da[j]->stop(mode));
  }
}

AudioStudioManager* g_audio_manager = AudioStudioManager::instance();