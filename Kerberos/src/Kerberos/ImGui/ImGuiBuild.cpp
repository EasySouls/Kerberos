#include "kbrpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui/backends/imgui_impl_glfw.cpp>
#include <imgui/backends/imgui_impl_opengl3.cpp>
#ifdef KBR_PLATFORM_WINDOWS
#include <imgui/backends/imgui_impl_dx11.cpp>
#endif
