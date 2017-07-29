#ifndef AUDIO_FMOD_SYSTEM_H
#define AUDIO_FMOD_SYSTEM_H

#include "fmod_studio.hpp"
#include <string>

extern const char* FMODErrorMessage[];

bool FMOD_CHECK_fn(FMOD_RESULT result, const char *file, int line);

#ifdef _DEBUG
#define FMOD_CHECK(_result) FMOD_CHECK_fn(_result, __FILE__, __LINE__)
#else
#define FMOD_CHECK(_result)
#endif // PUBLISH_BUILD

typedef FMOD_RESULT RESULT;

#endif	//AUDIO_FMOD_SYSTEM_H