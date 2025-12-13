#pragma once

#include <exception>
#include <source_location>
#include <string>

namespace Kerberos
{
	class Exception : public std::exception
	{
	public:
		explicit Exception(std::string message, const std::source_location& sourceLoc = std::source_location::current())
			: m_Message(std::move(message)),
			  m_File(sourceLoc.file_name()),
			  m_Line(static_cast<uint32_t>(sourceLoc.line())),
			  m_Function(sourceLoc.function_name())
		{
		}
			
	private:
		std::string m_Message;
		std::string m_File;
		uint32_t m_Line;
		std::string m_Function;
	};

	class NotImplementedException : public Exception
	{
		public:
		explicit NotImplementedException(const std::string& message = "Functionality not yet implemented.", const std::source_location& sourceLoc = std::source_location::current())
			: Exception(message, sourceLoc)
		{
		}
	};
}