#pragma once

#include "log.hpp"
#include "ytlib/misc/misc_macro.h"

#define CTX(x, ...) ytlib::Ctx(new std::map<std::string, std::string>{x, ##__VA_ARGS__})

#define YT_LOG(lvl, fmt, ...) Log::Ins().Trace(lvl, COMMON_FMT(fmt), ##__VA_ARGS__)

#define YT_TRACE(fmt, ...) YT_LOG(L_TRACE, fmt, ##__VA_ARGS__)
#define YT_DEBUG(fmt, ...) YT_LOG(L_DEBUG, fmt, ##__VA_ARGS__)
#define YT_INFO(fmt, ...) YT_LOG(L_INFO, fmt, ##__VA_ARGS__)
#define YT_WARN(fmt, ...) YT_LOG(L_WARN, fmt, ##__VA_ARGS__)
#define YT_ERROR(fmt, ...) YT_LOG(L_ERROR, fmt, ##__VA_ARGS__)
#define YT_FATAL(fmt, ...) YT_LOG(L_FATAL, fmt, ##__VA_ARGS__)

#define YT_LOG_C(lvl, ctx, fmt, ...) Log::Ins().Trace(lvl, ctx, COMMON_FMT(fmt), ##__VA_ARGS__)

#define YT_TRACE_C(ctx, fmt, ...) YT_LOG_C(L_TRACE, ctx, fmt, ##__VA_ARGS__)
#define YT_DEBUG_C(ctx, fmt, ...) YT_LOG_C(L_DEBUG, ctx, fmt, ##__VA_ARGS__)
#define YT_INFO_C(ctx, fmt, ...) YT_LOG_C(L_INFO, ctx, fmt, ##__VA_ARGS__)
#define YT_WARN_C(ctx, fmt, ...) YT_LOG_C(L_WARN, ctx, fmt, ##__VA_ARGS__)
#define YT_ERROR_C(ctx, fmt, ...) YT_LOG_C(L_ERROR, ctx, fmt, ##__VA_ARGS__)
#define YT_FATAL_C(ctx, fmt, ...) YT_LOG_C(L_FATAL, ctx, fmt, ##__VA_ARGS__)
