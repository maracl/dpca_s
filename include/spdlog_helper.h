#ifndef __SDPLOG_HELPER_H__
#define __SDPLOG_HELPER_H__

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/fmt/fmt.h"

#define LOG_GET(name) spdlog::get(name)

#define LOG_TRACE(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(name, ...) SPDLOG_LOGGER_CALL(LOG_GET(name), spdlog::level::critical, __VA_ARGS__)

#endif // !__SDPLOG_HELPER_H__
