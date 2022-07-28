#include "framework.h"
#include "Game/debug/debug.h"

#include <spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void InitTENLog()
{
	// "true" means that we create a new log file each time we run the game
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/TENLog.txt", true);

	std::shared_ptr<spdlog::logger> logger;

	// Set the file and console log targets
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	logger = std::make_shared<spdlog::logger>(std::string{ "multi_sink" }, spdlog::sinks_init_list{ file_sink, console_sink });
	
	spdlog::initialize_logger(logger);
    logger->set_level(spdlog::level::info);
	logger->flush_on(spdlog::level::info);   
	logger->set_pattern("[%Y-%b-%d %T] [%^%l%$] %v");
}

void TENLog(std::string_view str, LogLevel level, LogConfig config)
{
	static std::string lastString = {};

	if (lastString == str)
		return;

	if constexpr (!DebugBuild)
	{
		if (LogConfig::Debug == config)
			return;
	}

	auto logger = spdlog::get("multi_sink");
	switch (level)
	{
	case LogLevel::Error:
		logger->error(str);
	break;
	case LogLevel::Warning:
		logger->warn(str);
		break;
	case LogLevel::Info:
		logger->info(str);
		break;
	}

	logger->flush();

	lastString = std::string(str);
}

void ShutdownTENLog()
{
	spdlog::shutdown();
}
