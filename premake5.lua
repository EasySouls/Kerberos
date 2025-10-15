workspace "Kerberos"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

	startproject "KerberosEditor"

	flags
	{
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_DIR = os.getenv("VULKAN_SDK") or ""
if VULKAN_DIR == "" then
	VULKAN_DIR == "%{wks.location}/Kerberos/vendor/VulkanSDK/1.4.328.1"


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
IncludeDir["Mono"] = "%{wks.location}/Kerberos/vendor/mono/include"
IncludeDir["Filewatch"] = "%{wks.location}/Kerberos/vendor/filewatch"
IncludeDir["msdfgen"] = "%{wks.location}/Kerberos/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Kerberos/vendor/msdf-atlas-gen/msdf-atlas-gen"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_DIR}/Lib"
LibraryDir["Mono"] = "%{wks.location}/Kerberos/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["Mono"] = "%{LibraryDir.Mono}/libmono-static-sgen.lib"
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows-only
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"


-- Function to find the latest Windows SDK
function getLatestWindowsSDK()
	local windowsSdkDir = os.getenv("WindowsSdkDir")
	if windowsSdkDir == nil then
		windowsSdkDir = os.getenv("ProgramFiles(x86)") .. "/Windows Kits/10"
	end

	local includeBaseDir = windowsSdkDir .. "/Include/"
	local latestVersion = ""
	local latestVersionNum = 0

	local candidates = os.matchdirs(includeBaseDir .. "10.0.*")
	table.sort(candidates, function(a, b) return a > b end)
	for _, dir in ipairs(candidates) do
	  local version = path.getname(dir)
	  local potentialIncludeDir = includeBaseDir .. version .. "/um"
	  local potentialLibDir = windowsSdkDir .. "/Lib/" .. version .. "/um/x64"
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
	include "Kerberos/vendor/msdf-atlas-gen"
	include "Kerberos/vendor/ImGuizmo"
	include "Kerberos/vendor/Assimp"
	include "Kerberos/vendor/JoltPhysics"
group ""

group "Core"
	include "KerberosScriptCoreLib"
group ""

project "Kerberos"
	location "Kerberos"
	kind "StaticLib"
	staticruntime "off"
	language "C++"
	cppdialect "C++23"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "kbrpch.h"
	pchsource "Kerberos/src/kbrpch.cpp"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.ixx",

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
		IncludeDir.Mono,
		IncludeDir.Filewatch,
		IncludeDir.msdfgen,
		IncludeDir.msdf_atlas_gen,
	}

	libdirs 
	{
		LibraryDir.VulkanSDK,
		LibraryDir.Mono
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"msdf-atlas-gen",
		"ImGuizmo",
		"Assimp",
		"JoltPhysics",

		"KerberosScriptCoreLib",

		"opengl32.lib",
		Library.Vulkan,
		Library.Mono,
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
			"dxguid.lib",

			Library.WinSock,
			Library.WinMM,
			Library.WinVersion,
			Library.BCrypt,
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
	cppdialect "C++23"
	
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
	cppdialect "C++23"
	
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