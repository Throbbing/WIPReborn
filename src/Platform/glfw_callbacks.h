#pragma once
#include "../../libs/glfw/include/GLFW/glfw3.h"

void glfw_error_callback(int error,const char* description);

void glfw_mouse_pos_callback(GLFWwindow* windows,double x,double y);

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void glfw_scroll_callback(GLFWwindow* window, double x, double y);

void glfw_char_callback(GLFWwindow* window, unsigned int c);