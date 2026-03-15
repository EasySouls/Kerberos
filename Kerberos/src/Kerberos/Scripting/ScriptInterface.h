#pragma once

namespace Kerberos
{
	class ScriptInterface
	{
	public:
		/// Registers native function pointers with the managed scripting system.
		/// This replaces the old Mono internal call registration.
		static void RegisterFunctions();
	};
}