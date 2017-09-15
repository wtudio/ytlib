project( RPSF_CenterNode )
#中心节点

File(GLOB SRC_Files *.cpp *.h ${ytlib_SOURCE_DIR}/ytrpsf/*.h )


add_executable( RPSF_CenterNode ${SRC_Files} )

IF(WIN32)
target_link_libraries(RPSF_CenterNode sqlite sigar )
ELSE(WIN32)
target_link_libraries(RPSF_CenterNode sqlite sigar -lboost_system -lboost_chrono -lboost_thread -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_serialization -lboost_atomic -lpthread)
ENDIF(WIN32)


