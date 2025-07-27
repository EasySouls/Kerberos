#include "kbrpch.h"

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Kerberos
{
	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		// [Timestamp] [name of logger]: [message]
		spdlog::set_pattern("%^[%T] %n: %v%$");

		auto coreConsoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		coreConsoleSink->set_level(spdlog::level::trace);
		auto coreFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Kerberos.log", true);
		coreFileSink->set_level(spdlog::level::trace);

		s_CoreLogger = CreateRef<spdlog::logger>(spdlog::logger("KERBEROS", { coreConsoleSink, coreFileSink }));
		s_CoreLogger->set_level(spdlog::level::info);

		auto clientConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		clientConsoleSink->set_level(spdlog::level::trace);
		auto clientFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("KerberosClient.log", true);
		clientFileSink->set_level(spdlog::level::trace);

		s_ClientLogger = CreateRef<spdlog::logger>(spdlog::logger("APP", { clientConsoleSink, clientFileSink }));
		s_ClientLogger->set_level(spdlog::level::info);
	}
}