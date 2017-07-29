#ifndef SOUND_H
#define SOUND_H

#include "fmod_studio.hpp"
#include "AudioFmodSystem.h"
#include <vector>
#include "Vector3.h"
#include "Logger.h"

class StudioSound
{
private:
    class DescriptionData
	{
    public:
      DescriptionData(FMOD::Studio::EventDescription* desc) :
        desc_ref(desc){
        FMOD_CHECK(desc_ref->getLength(&event_length));
        FMOD_CHECK(desc_ref->getSoundSize(&sound_size));
        FMOD_CHECK(desc_ref->getMaximumDistance(&max_dist));
        FMOD_CHECK(desc_ref->getMinimumDistance(&min_dist));
        FMOD_CHECK(desc_ref->is3D(&is_3d));
        FMOD_CHECK(desc_ref->isOneshot(&is_one_shot));
        FMOD_CHECK(desc_ref->isStream(&is_stream));
      }
      ~DescriptionData(){
        FMOD_CHECK(desc_ref->releaseAllInstances());
      }
      void StopAll(FMOD_STUDIO_STOP_MODE mode = FMOD_STUDIO_STOP_IMMEDIATE);
    private:
      FMOD::Studio::EventDescription* desc_ref;
      friend class StudioSound;
      friend class AudioStudioManager;
    public:
      int event_length;
      float max_dist;
      float min_dist;
      float sound_size;
      bool is_3d;
      bool is_one_shot;
      bool is_stream;
    };
	typedef DescriptionData* DescriptionDataRef;
public:
  /**** Parameter ****/
  FMOD_STUDIO_PARAMETER_DESCRIPTION GetParamDesc(const std::string& param_name);
  void PrintAllParam();
  int GetParamCount();
  float GetParam(const std::string& param_name);
  void SetParam(const std::string& param_name, float val);
  //return count
  int GetAllParams(std::vector<std::string>& outv);
  bool IsValid();
  bool IsVirtual();
  /***** Event ****/
  RBVector3 Get3DPosition();
  void Set3DPosition(const RBVector3& val);
  RBVector3 Get3DVelocity();
  void Set3DVelocity(const RBVector3& val);
  RBVector3 Get3DUp();
  void Set3DUp(const RBVector3& val);
  RBVector3 Get3DForward();
  void Set3DForward(const RBVector3& val);
  FMOD::Studio::EventDescription* GetEventDesc();
  bool IsPause();
  void Pause();
  void Resume();
  bool IsPlaying();
  float GetPitch();
  void SetPitch(float val);
  int GetTimelinePosition();
  void SetTimelinePosition(int position);
  float GetVolume();
  void SetVolume(float val);
  /**** Callback ***/
  void SetCallback(FMOD_STUDIO_EVENT_CALLBACK callback, int mask);
  void CallBackTest(){ LOG_NOTE("CallBack->Event:%d Name:%s\n",_inst,_name.c_str()); };
  void SetCallback(const std::string& callback_name,int callback_type );
  /**** Event Description ****/
  void LoadSample();
  void UnLoadSamlpe();
  StudioSound::DescriptionDataRef desc_ref;
  /** Event Property **/
  float GetChannelPriority();
  float GetScheduleDelay();
  float GetScheduleLookahead();
  float GetMinDist();
  float GetMaxDist();
  void SetChannelPriority(float val);
  void SetScheduleDelay(float val);
  void SetScheduleLookahead(float val);
  void SetMinDist(float val);
  void SetMaxDist(float val);
  //not use
  void SetUserData(void *user_data);
  void* GetUserData();

  void SetStudioSound(StudioSound* userdata);
  StudioSound* GetStudioSound();
private:
  void DeleteDelay();
  void Start();
  void Stop(FMOD_STUDIO_STOP_MODE mode = FMOD_STUDIO_STOP_IMMEDIATE);
  std::string _path;
  const std::string _name;
  bool _is_playing;
  std::string _started_call_back;
  std::string _restarted_call_back;
  std::string _stopped_call_back;
  FMOD::Studio::EventInstance* _inst;

  class AudioStudioManager& _manager;
  friend class AudioStudioManager;

  //event callback
  static FMOD_RESULT F_CALLBACK EventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters);

  static const int MAX_STR_LEN;
public:
  StudioSound(AudioStudioManager& manager, StudioSound::DescriptionDataRef desc, const std::string& name);
  ~StudioSound();
};

typedef StudioSound* StudioSoundRef;

//not use
class Bus
{
public:
  float GetFaderLevel();
  bool GetMute();
  bool GetPaused();

  void SetFaderLevel(float val);
  void SetMute(bool val);
  void SetPaused(bool val);

  void StopAllEvents(FMOD_STUDIO_STOP_MODE mode);

  const std::string _name;
protected:
  Bus(AudioStudioManager& manager, const std::string& path);
  ~Bus();

private:
  FMOD::Studio::Bus* _bus;
  class AudioStudioManager& _manager;
  friend class AudioStudioManager;
};
#endif // SOUND_H