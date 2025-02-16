workspace "Kerberos"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

	startproject "Sandbox"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_DIR = os.getenv("VULKAN_SDK")

-- Include directories relative to the solution directory
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Kerberos/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Kerberos/vendor/glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Kerberos/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Kerberos/vendor/glm"
IncludeDir["VulkanSDK"] = "%{VULKAN_DIR}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_DIR}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

-- Include GLFW premake file
include "Kerberos/vendor/GLFW"
include "Kerberos/vendor/glad"
include "Kerberos/vendor/imgui"

project "Kerberos"
	location "Kerberos"
	kind "StaticLib"
	staticruntime "on"
	language "C++"
	cppdialect "C++20"

	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "kbrpch.h"
	pchsource "Kerberos/src/kbrpch.cpp"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		IncludeDir.GLFW,
		IncludeDir.Glad,
		IncludeDir.ImGui,
		IncludeDir.glm,
		IncludeDir.VulkanSDK
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE",
			"_WINDLL",
			"IMGUI_DOCKING_BRANCH"
		}
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"
		
	filter "configurations:Dist"
		defines "KB_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	staticruntime "on"
	language "C++"
	cppdialect "C++20"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"Kerberos/vendor",
		"Kerberos/src",
		IncludeDir.glm
	}
	
	links
	{
		"Kerberos",
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		symbols "on"
		
	filter "configurations:Release"
		defines "KBR_RELEASE"
		optimize "on"
		
	filter "configurations:Dist"
		defines "KBR_DIST"
		optimize "on"