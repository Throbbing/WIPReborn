#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "Sound.h"
#include "ProducerConsumerQueue.h"
#include <map>
#include "RefCount.h"

class AudioStudioManager : public FRefCountedObject
{
private:
  class CallbackEventData
  {
  public:
    CallbackEventData(FMOD_STUDIO_EVENT_CALLBACK_TYPE ttype, FMOD_STUDIO_EVENTINSTANCE *tevent, void *tparameters) :
      type(ttype), event(tevent), parameters(tparameters){}
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type;
    FMOD_STUDIO_EVENTINSTANCE* event;
    void* parameters;
  };
  typedef CallbackEventData* CallbackEventDataRef;
public:
  static const int MAX_CHANNEL_COUNT;
  static const int MAX_BUS_COUNT;
  static const int MAX_INSTANCE_COUNT;
  static const int MAX_QUEUE_SIZE;

  static AudioStudioManager* instance();

  AudioStudioManager();
  ~AudioStudioManager();

  void Update();


  void Play(StudioSoundRef sound);
  void Play(const std::string& event_name);
  void Stop(StudioSoundRef sound, FMOD_STUDIO_STOP_MODE mode = FMOD_STUDIO_STOP_IMMEDIATE);
  void Stop(const std::string& event_name, FMOD_STUDIO_STOP_MODE mode = FMOD_STUDIO_STOP_IMMEDIATE);
  void StopAll(FMOD_STUDIO_STOP_MODE mode = FMOD_STUDIO_STOP_IMMEDIATE);
  
  /*
  Todo:
  Check loaded functions
  */

  /*** Studio ***/
  StudioSoundRef CreateSound(const std::string& sound_name, const std::string& event_path);
  StudioSoundRef CreateSound(StudioSound::DescriptionDataRef desc);
  StudioSoundRef CreateSound(const std::string& event_name);
  void DeleteSound(StudioSoundRef sound);
  Bus* CreateBus(const std::string& bus_name, const std::string& path) = delete;

  void ClearAllSound();

  bool LoadBank(const std::string& filename, bool async = false);
  void UnloadAllBanks();
  void LoadAllBuses();
  void ReleaseSoundInstance(StudioSoundRef sound);

  void FlushFmodCommands();
  FMOD_STUDIO_CPU_USAGE GetCPUUsage();

  const char* GetLastError();
  void SetLastError(FMOD_RESULT rescode);

  /**** Listener ****/
  void SetListenerPosition(int index, const RBVector3& p);
  RBVector3 GetListenerPosition(int index);
  void SetListenerVelocity(int index, const RBVector3& v);
  RBVector3 GetListenerVelocity(int index);
  void SetListenerUp(int index, const RBVector3& up);
  RBVector3 GetListenerUp(int index);
  void SetListenerForward(int index, const RBVector3& forward);
  RBVector3 GetListenerForward(int index);

  /**** Record ****/
  void StartCommandCapture(const std::string& recordfile);
  void StopCommandCapture();

  /**** Bus ****/
  float GetFaderLevel(const std::string& name);
  bool GetMute(const std::string& name);
  bool GetPaused(const std::string& name);
  void SetFaderLevel(const std::string& name,float val);
  void SetMute(const std::string& name,bool val);
  void SetPaused(const std::string& name,bool val);
  void StopAllEvents(const std::string& name,FMOD_STUDIO_STOP_MODE mode);

   /** Bank **/


   /** Description **/
  StudioSound::DescriptionDataRef GetDescription(const std::string& desc_name);
  int GetInstanceCount(const std::string& desc_name);


private:
  inline FMOD::Studio::Bus* GetBus(const std::string& name)
  {
    auto it = _buses.find(name);
    if (it == _buses.end()){
      FMOD::Studio::Bus* bus = nullptr;
      FMOD_CHECK(_studio_system->getBus(name.c_str(), &bus));
      _buses.insert(FmodeBusMap::value_type(name, bus));
      return bus;
    }
    else{
      return it->second;
    }
  }

  bool CheckBankState();
  void SetDefaultCallbacks(FMOD::Studio::EventDescription* desc);

  FMOD::Studio::System* _studio_system;
  FMOD::System* _lowlevel_system;

  typedef vector<FMOD::Studio::Bank*> SoundVector;
  SoundVector _bank_vector;

  typedef map<std::string, StudioSound::DescriptionDataRef> FmodEventMap;
  FmodEventMap _event_desc_map;

  //the alias of a event path 
  typedef map<std::string, std::string> PathAliasMap;
  PathAliasMap _path_alias_map;

  typedef map<std::string, FMOD::Studio::Bus*> FmodeBusMap;
  FmodeBusMap _buses;

  FMOD_RESULT _last_error;

  map<FMOD::Studio::EventInstance*,StudioSound*> _delete_delay;

  /*
  typedef unordered_map<String, StudioSound*> StudioSoundMap;
  StudioSoundMap _studio_sound_playing_map;
  */
  folly::ProducerConsumerQueue<CallbackEventData*> _event_queue;


  friend class StudioSound;
  friend class Bus;

  //default callback
  static FMOD_RESULT F_CALLBACK DefaultEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters);

};

extern AudioStudioManager* g_audio_manager;


#endif // AUDIO_MANAGER_H
