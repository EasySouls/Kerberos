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
IncludeDir["stb_image"] = "%{wks.location}/Kerberos/vendor/stb_image"
IncludeDir["VulkanSDK"] = "%{VULKAN_DIR}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_DIR}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

group "Dependencies"
	include "Kerberos/vendor/GLFW"
	include "Kerberos/vendor/glad"
	include "Kerberos/vendor/imgui"
group ""

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

		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp"
	}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		IncludeDir.GLFW,
		IncludeDir.Glad,
		IncludeDir.ImGui,
		IncludeDir.glm,
		IncludeDir.stb_image,
		IncludeDir.VulkanSDK
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"IMGUI_DOCKING_BRANCH"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS",
			"_WINDLL",
		}

		local windowsSdkDir = os.getenv("WindowsSdkDir")
		print(windowsSdkDir)
		if windowsSdkDir == nil then
			windowsSdkDir = os.getenv("ProgramFiles(x86)") .. "/Windows Kits/10"
		end

		local windowsSdkIncludeDir = windowsSdkDir .. "/Include/10.0.22621.0/um"
		local windowsSdkLibDir = windowsSdkDir .. "/Lib/10.0.22621.0/um/x64"

		includedirs
		{
			windowsSdkIncludeDir
		}

		libdirs
		{
			windowsSdkLibDir
		}

		links
		{
			--[["User32.lib",
			"kernel32.lib",
			"gdi32.lib",
			"winspool.lib",
			"comdlg32.lib",
			"advapi32.lib",
			"shell32.lib",
			"ole32.lib",
			"oleaut32.lib",
			"uuid.lib",
			"odbc32.lib",
			"odbccp32.lib",
			"Vulkan",
			"VulkanUtils"]]--
			"d3d11.lib",
			"dxgi.lib",
			"d3dcompiler.lib"
		}

		filter "system:linux"
		systemversion "latest"
		
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