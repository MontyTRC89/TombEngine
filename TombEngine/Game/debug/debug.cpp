#include "framework.h"
#include "Game/debug/debug.h"

#include <spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace TEN::Debug
{
	void InitTENLog(const std::string& logDirContainingDir)
	{
		// "true" means create new log file each time game is run.
		auto logPath = logDirContainingDir + "Logs/TENLog.txt";
		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath, true);

		auto logger = std::shared_ptr<spdlog::logger>();

		// Set file and console log targets.
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		logger = std::make_shared<spdlog::logger>(std::string("multi_sink"), spdlog::sinks_init_list{ fileSink, consoleSink });

		spdlog::initialize_logger(logger);
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		logger->set_pattern("[%Y-%b-%d %T] [%^%l%$] %v");
	}

	void ShutdownTENLog()
	{
		spdlog::shutdown();
	}

	void TENLog(const std::string_view& string, LogLevel level, LogConfig config, bool allowSpam)
	{
		static auto prevString = std::string();

		if (prevString == string && !allowSpam)
			return;

		if constexpr (!DebugBuild)
		{
			if (config == LogConfig::Debug)
				return;
		}

		auto logger = spdlog::get("multi_sink");
		switch (level)
		{
		case LogLevel::Error:
			logger->error(string);
			break;

		case LogLevel::Warning:
			logger->warn(string);
			break;

		case LogLevel::Info:
			logger->info(string);
			break;
		}

		logger->flush();

		prevString = std::string(string);
	}
}
