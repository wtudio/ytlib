# ytlib开发测试专用

add_subdirectory(log)

add_subdirectory(testlib)
add_subdirectory(testlib2)
add_subdirectory(testprj)

# add_subdirectory(testytrpc)

add_subdirectory(testexecutor)
add_subdirectory(testboostfiber)
add_subdirectory(testboostasio)

add_subdirectory(testthreadpool)

# ---------------------------------

include(GetHiRedis)
include(GetLibevent)

add_subdirectory(testredis)

# ---------------------------------

include(GetGoogleBenchmark)

add_subdirectory(custom_bench)

# ---------------------------------

include(GetFreeglut)

add_subdirectory(testgame)
add_subdirectory(testgl)

include(GetEntt)
include(GetRaylib)

add_subdirectory(testgame2)

# ---------------------------------

include(GetNlohmannJson)
include(GetProtoBuf)
include(GetOpenTelemetryProto)
include(GetOpenTelemetryCpp)

add_subdirectory(testot)
