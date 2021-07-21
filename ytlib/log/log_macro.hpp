#pragma once

#include "log.hpp"
#include "ytlib/misc/misc_macro.h"

#define CTX(x, ...) ytlib::Ctx(new std::map<std::string, std::string>{x, ##__VA_ARGS__})

#define LOG_FMT(fmt) "[" __FILENAME__ ":" STRING(__LINE__) "@%s]" fmt, __FUNCTION__

#define YT_LOG(lvl, fmt, ...) Log::Ins().Trace(lvl, LOG_FMT(fmt), ##__VA_ARGS__)

#define YT_TRACE(fmt, ...) YT_LOG(LOG_LEVEL::L_TRACE, fmt, ##__VA_ARGS__)
#define YT_DEBUG(fmt, ...) YT_LOG(LOG_LEVEL::L_DEBUG, fmt, ##__VA_ARGS__)
#define YT_INFO(fmt, ...) YT_LOG(LOG_LEVEL::L_INFO, fmt, ##__VA_ARGS__)
#define YT_WARN(fmt, ...) YT_LOG(LOG_LEVEL::L_WARN, fmt, ##__VA_ARGS__)
#define YT_ERROR(fmt, ...) YT_LOG(LOG_LEVEL::L_ERROR, fmt, ##__VA_ARGS__)
#define YT_FATAL(fmt, ...) YT_LOG(LOG_LEVEL::L_FATAL, fmt, ##__VA_ARGS__)

#define YT_LOG_C(lvl, ctx, fmt, ...) Log::Ins().Trace(lvl, ctx, LOG_FMT(fmt), ##__VA_ARGS__)

#define YT_TRACE_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_TRACE, ctx, fmt, ##__VA_ARGS__)
#define YT_DEBUG_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_DEBUG, ctx, fmt, ##__VA_ARGS__)
#define YT_INFO_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_INFO, ctx, fmt, ##__VA_ARGS__)
#define YT_WARN_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_WARN, ctx, fmt, ##__VA_ARGS__)
#define YT_ERROR_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_ERROR, ctx, fmt, ##__VA_ARGS__)
#define YT_FATAL_C(ctx, fmt, ...) YT_LOG_C(LOG_LEVEL::L_FATAL, ctx, fmt, ##__VA_ARGS__)
