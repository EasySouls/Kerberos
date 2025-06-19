workspace "Kerberos"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

	startproject "KerberosEditor"

	flags
	{
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_DIR = os.getenv("VULKAN_SDK")

print("Vulkan SDK Directory: " .. VULKAN_DIR)

-- Include directories relative to the solution directory
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Kerberos/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Kerberos/vendor/glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Kerberos/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Kerberos/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Kerberos/vendor/stb_image"
IncludeDir["VulkanSDK"] = "%{VULKAN_DIR}/Include"
IncludeDir["entt"] = "%{wks.location}/Kerberos/vendor/entt/Include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Kerberos/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Kerberos/vendor/ImGuizmo"
IncludeDir["Assimp"] = "%{wks.location}/Kerberos/vendor/Assimp/include"
IncludeDir["JoltPhysics"] = "%{wks.location}/Kerberos/vendor/JoltPhysics"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_DIR}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Function to find the latest Windows SDK
function getLatestWindowsSDK()
	local windowsSdkDir = os.getenv("WindowsSdkDir")
	if windowsSdkDir == nil then
		windowsSdkDir = os.getenv("ProgramFiles(x86)") .. "/Windows Kits/10"
	end

	local includeBaseDir = windowsSdkDir .. "/Include/"
	local latestVersion = ""
	local latestVersionNum = 0

	-- Premake doesn't have os.walk(), so we'll rely on checking common SDK locations
	-- This is less robust than walking directories but more likely to work with Premake
	local commonSdkVersions =
	{
		"10.0.26100.0",
		"10.0.22621.0", -- Windows 11 22H2/23H2
		"10.0.22000.0",
		"10.0.19041.0", -- Windows 10 2004/20H2/21H1/21H2
		"10.0.18362.0", -- Windows 10 1903/1909
		-- Add other recent versions if needed
	}

	-- Check for the existence of common SDK versions in descending order
	for _, version in ipairs(commonSdkVersions) do
		local potentialIncludeDir = includeBaseDir .. version .. "/um"
		local potentialLibDir = windowsSdkDir .. "/Lib/" .. version .. "/um/x64" -- Assuming x64

		if os.isdir(potentialIncludeDir) and os.isdir(potentialLibDir) then
			local buildNumStr = version:match("10%.0%.(%d+)%.0")
			local buildNum = tonumber(buildNumStr)
			if buildNum and buildNum > latestVersionNum then
				latestVersionNum = buildNum
				latestVersion = version
			end
		end
	end

	if latestVersion == "" then
		print("Warning: Could not find a suitable Windows SDK version among common versions.")
		print("Please ensure a supported Windows 10/11 SDK is installed.")
		return nil, nil
	end

	local includeDir = includeBaseDir .. latestVersion .. "/um"
	local libDir = windowsSdkDir .. "/Lib/" .. latestVersion .. "/um/x64"

	return includeDir, libDir
end

group "Dependencies"
	include "Kerberos/vendor/GLFW"
	include "Kerberos/vendor/glad"
	include "Kerberos/vendor/imgui"
	include "Kerberos/vendor/yaml-cpp"
	include "Kerberos/vendor/ImGuizmo"
	include "Kerberos/vendor/Assimp"
	include "Kerberos/vendor/JoltPhysics"
group ""

project "Kerberos"
	location "Kerberos"
	kind "StaticLib"
	staticruntime "off"
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
		"%{prj.name}/vendor/stb_image/**.cpp",
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
		IncludeDir.VulkanSDK,
		IncludeDir.entt,
		IncludeDir.yaml_cpp,
		IncludeDir.ImGuizmo,
		IncludeDir.Assimp,
		IncludeDir.JoltPhysics,
	}

	libdirs 
	{
		LibraryDir.VulkanSDK
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"ImGuizmo",
		"Assimp",
		"JoltPhysics",

		"opengl32.lib",
		Library.Vulkan,
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"IMGUI_DOCKING_BRANCH",
		"YAML_CPP_STATIC_DEFINE",
	}

	filter "files:vendor/ImGuizmo/ImGuizmo.cpp"
		flags { "NoPCH" }
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"KBR_PLATFORM_WINDOWS",
		}

		local windowsSdkIncludeDir, windowsSdkLibDir = getLatestWindowsSDK()
		print("Windows SDK Include dir: " .. windowsSdkIncludeDir)
		print("Windows SDK Lib dir: " .. windowsSdkLibDir)

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
			"odbccp32.lib"]]--
			"d3d11.lib",
			"dxgi.lib",
			"d3dcompiler.lib",
			"winmm.lib",
			"dxguid.lib"
		}

	filter "system:linux"
		systemversion "latest"
		
	filter "configurations:Debug"
		defines "KBR_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			Library.ShaderC_Debug,
			Library.SPIRV_Cross_Debug,
			Library.SPIRV_Cross_GLSL_Debug,
	--		Library.SPIRV_Tools_Debug,
		}
		
	filter "configurations:Release"
		defines "KBR_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			Library.ShaderC_Release,
			Library.SPIRV_Cross_Release,
			Library.SPIRV_Cross_GLSL_Release,
		}
		
	filter "configurations:Dist"
		defines "KBR_DIST"
		runtime "Release"
		optimize "on"

		links 
		{
			Library.ShaderC_Release,
			Library.SPIRV_Cross_Release,
			Library.SPIRV_Cross_GLSL_Release,
		}

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	staticruntime "off"
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
		"Kerberos/src",
		"Kerberos/vendor",
		"Kerberos/vendor/spdlog/include",

		IncludeDir.glm,
		IncludeDir.entt,
		--IncludeDir.Assimp,
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

project "KerberosEditor"
	location "KerberosEditor"
	kind "ConsoleApp"
	staticruntime "off"
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
		"Kerberos/src",
		"Kerberos/vendor",
		"Kerberos/vendor/spdlog/include",

		IncludeDir.glm,
		IncludeDir.entt,
		--IncludeDir.Assimp
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