#include "kbrpch.h"

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Kerberos
{
	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	/**
	 * @brief Initializes the logging subsystem and configures core and client loggers.
	 *
	 * Sets the global log format to "[HH:MM:SS] logger_name: message", creates colorized stdout and file sinks
	 * for both the core ("KERBEROS") and client ("APP") loggers, and assigns them to the corresponding
	 * global logger references.
	 *
	 * - Console sinks write to stdout and have their sink-level set to `trace`.
	 * - File sinks write to "Kerberos.log" (core) and "KerberosClient.log" (client) in append mode and have
	 *   their sink-level set to `trace`.
	 * - Both global logger objects (`s_CoreLogger`, `s_ClientLogger`) are created with their sinks and
	 *   have their logger-level set to `info`.
	 */
	void Log::Init()
	{
		// [Timestamp] [name of logger]: [message]
		spdlog::set_pattern("%^[%T] %n: %v%$");

		auto coreConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
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