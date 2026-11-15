#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#if VISLAM_HAS_SPDLOG
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#endif

namespace vislam::utils {

enum class LogLevel {
	kInfo,
	kWarning,
	kError
};

inline const char* ToString(const LogLevel level) {
	switch (level) {
		case LogLevel::kInfo:
			return "INFO";
		case LogLevel::kWarning:
			return "WARN";
		case LogLevel::kError:
			return "ERROR";
		default:
			return "UNKNOWN";
	}
}

inline void Log(const LogLevel level, const std::string& message) {
#if VISLAM_HAS_SPDLOG
	auto logger = spdlog::get("vislam");
	if (!logger) {
		logger = spdlog::stdout_color_mt("vislam");
		logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
	}

	switch (level) {
		case LogLevel::kInfo:
			logger->info(message);
			break;
		case LogLevel::kWarning:
			logger->warn(message);
			break;
		case LogLevel::kError:
			logger->error(message);
			break;
		default:
			logger->info(message);
			break;
	}
#else
	static std::mutex log_mutex;
	const auto now = std::chrono::system_clock::now();
	const auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm_value {};

#if defined(_WIN32)
	localtime_s(&tm_value, &now_time_t);
#else
	localtime_r(&now_time_t, &tm_value);
#endif

	std::ostringstream ss;
	ss << std::put_time(&tm_value, "%Y-%m-%d %H:%M:%S");

	std::lock_guard<std::mutex> lock(log_mutex);
	std::cout << "[" << ss.str() << "]"
			  << " [" << ToString(level) << "] " << message << '\n';
#endif
}

}  // namespace vislam::utils
