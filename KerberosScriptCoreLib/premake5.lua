project "KerberosScriptCoreLib"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	namespace "Kerberos"

	targetdir ("%{wks.location}/KerberosEditor/Resources/Scripts")
	objdir ("%{wks.location}/KerberosEditor/Resources/Scripts/Intermediates")

	files 
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "configurations:Debug"
		runtime "Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		runtime "Release"
		optimize "Full"
		symbols "Off"
