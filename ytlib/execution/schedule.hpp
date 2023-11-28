#pragma once

#include <unifex/scheduler_concepts.hpp>

namespace ytlib {

inline constexpr auto& Schedule = unifex::schedule;
inline constexpr auto& ScheduleAfter = unifex::schedule_after;
inline constexpr auto& ScheduleAt = unifex::schedule_at;

}  // namespace ytlib
