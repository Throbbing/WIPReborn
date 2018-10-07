#include "AppManager.h"
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

WIPApp::WIPApp()
{
	g_animation_manager = WIPAnimationManager::instance();
	g_audio_manager = AudioStudioManager::instance();
	g_filesystem = WIPFileSystem::get_instance();
	g_logger = WIPLogger::get_instance();
	g_res_manager = WIPResourceManager::get_instance();
	g_input_manager = InputManager::instance();
	g_mem_manager = WIPMemoryManager::get_instance();
	g_pool_allocator = RBPoolAllctor::instance();
	g_scene = WIPScene::instance();
	g_physics_manager = WIPPhysicsManager::instance();
	g_script_manager = WIPScriptManager::instance();
	g_rhi = WIPDynamicRHI::get_rhi();
}

WIPApp::~WIPApp()
{
	g_animation_manager->Reduce();
	g_animation_manager->Release();
	g_audio_manager->Reduce();
	g_audio_manager->Release();
	g_filesystem->Reduce();
	g_filesystem->Release();
	g_logger->Reduce();
	g_logger->Release();
	g_res_manager->Reduce();
	g_res_manager->Release();
	g_input_manager->Reduce();
	g_input_manager->Release();
	g_mem_manager->Reduce();
	g_mem_manager->Release();
	g_pool_allocator->Reduce();
	g_pool_allocator->Release();
	g_scene->Reduce();
	g_scene->Release();
	g_physics_manager->Reduce();
	g_physics_manager->Release();
	g_script_manager->Reduce();
	g_script_manager->Release();
	g_rhi->Reduce();
	g_rhi->Release();

	g_animation_manager = nullptr;
	g_audio_manager = nullptr;
	g_filesystem = nullptr;
	g_logger = nullptr;
	g_res_manager = nullptr;
	g_input_manager = nullptr;
	g_mem_manager = nullptr;
	g_pool_allocator = nullptr;
	g_scene = nullptr;
	g_physics_manager = nullptr;
	g_script_manager = nullptr;
	g_rhi = nullptr;
}
