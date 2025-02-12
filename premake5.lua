workspace "Kerberos"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

	startproject "Sandbox"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS",
			"KBR_BUILD_DLL",
			"_WINDLL"
		}
		
		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines "KB_RELEASE"
		optimize "On"
		
	filter "configurations:Dist"
		defines "KB_DIST"
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
		"Kerberos"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
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