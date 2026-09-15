#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

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
}

}  // namespace vislam::utils
