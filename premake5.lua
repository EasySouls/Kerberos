workspace "Kerberos"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

	startproject "Sandbox"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to the solution directory
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Kerberos/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Kerberos/vendor/glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Kerberos/vendor/imgui"

-- Include GLFW premake file
include "Kerberos/vendor/GLFW"
include "Kerberos/vendor/glad"
include "Kerberos/vendor/imgui"

project "Kerberos"
	location "Kerberos"
	kind "SharedLib"
	language "C++"
	
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
		IncludeDir.ImGui
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS",
			"KBR_BUILD_DLL",
			"KBR_ENABLE_ASSERTS",
			"GLFW_INCLUDE_NONE",
			"_WINDLL",
			"IMGUI_DOCKING_BRANCH"
		}
		
		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		buildoptions "/MDd"
		symbols "On"
		
	filter "configurations:Release"
		defines "KB_RELEASE"
		buildoptions "/MD"
		optimize "On"
		
	filter "configurations:Dist"
		defines "KB_DIST"
		buildoptions "/MD"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	
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
		"Kerberos/src"
	}
	
	links
	{
		"Kerberos",
	}
	
	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines "KBR_RELEASE"
		optimize "On"
		
	filter "configurations:Dist"
		defines "KBR_DIST"
		optimize "On"