#include "AnimationManager.h"
#include "AudioManager.h"
#include "FileSystem.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "PhysicsManager.h"
#include "InputManager.h"
#include "Scene.h"
#include "ScriptManager.h"
#include "Render.h"

//do not run code out of main code! it's out of controll
WIPAnimationManager* g_animation_manager=nullptr;
AudioStudioManager* g_audio_manager=nullptr;
WIPFileSystem* g_filesystem=nullptr;
WIPLogger* g_logger=nullptr;
WIPResourceManager* g_res_manager=nullptr;
InputManager* g_input_manager=nullptr;
WIPMemoryManager* g_mem_manager=nullptr;
RBPoolAllctor* g_pool_allocator=nullptr;
WIPScene* g_scene=nullptr;
WIPPhysicsManager* g_physics_manager=nullptr;
WIPScriptManager* g_script_manager=nullptr;
WIPDynamicRHI *g_rhi=nullptr;