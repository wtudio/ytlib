
add_subdirectory(cache)
add_subdirectory(container)
add_subdirectory(dll_tools)
add_subdirectory(file)
add_subdirectory(logic)
add_subdirectory(math)
add_subdirectory(misc)
add_subdirectory(string)
add_subdirectory(thread)

if(YTLIB_BUILD_WITH_BOOST)
  add_subdirectory(boost_tools_asio)
  add_subdirectory(boost_tools_fiber)
  add_subdirectory(boost_tools_util)
endif()

if(YTLIB_BUILD_WITH_PROTOBUF)
  add_subdirectory(pb_tools)
endif()

if(YTLIB_BUILD_WITH_FREEGLUT)
  add_subdirectory(gl)
endif()

if(YTLIB_BUILD_WITH_LIBUNIFEX AND YTLIB_BUILD_WITH_BOOST)
  add_subdirectory(execution)
endif()

if(YTLIB_BUILD_WITH_LIBUNIFEX AND YTLIB_BUILD_WITH_BOOST AND YTLIB_BUILD_WITH_PROTOBUF)
  add_subdirectory(ytrpc)
endif()
