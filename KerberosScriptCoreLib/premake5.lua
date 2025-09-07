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
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"
