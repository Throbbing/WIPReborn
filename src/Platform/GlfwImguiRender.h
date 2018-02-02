#pragma once
//this is a lowlevel render.
//see also Platform/glfw_callbacks.cpp
#include "imgui/imgui.h"
#include "RefCount.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN32


class GlfwImguiRender : public FRefCountedObject
{
public:
	bool imgui_init(struct GLFWwindow* window, const char* ttf, int size_in_pixels);
	void imgui_shutdown();
	void imgui_new_frame();


	void imgui_invalidte_device_objects();
	bool imgui_create_device_objects(const char* ttf, int size_in_pixels);
	bool imgui_create_fonts_texture(const char* ttf, int size_in_pixels);

	
	static void ImGui_ImplGlfwGL3_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void ImGui_ImplGlfwGL3_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void ImGui_ImplGlfwGL3_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void ImGui_ImplGlfwGL3_CharCallback(GLFWwindow* window, unsigned int c);

	static void ImGui_ImplGlfwGL3_RenderDrawLists(ImDrawData* draw_data);




	static GLFWwindow*  g_Window;
	static double       g_Time;
	static bool         g_MousePressed[3];
	static float        g_MouseWheel;
	static unsigned int       g_FontTexture;
	static int          g_ShaderHandle, g_VertHandle, g_FragHandle;
	static int          g_AttribLocationTex, g_AttribLocationProjMtx;
	static int          g_AttribLocationPosition, g_AttribLocationUV, g_AttribLocationColor;
	static unsigned int g_VboHandle, g_VaoHandle, g_ElementsHandle;
};