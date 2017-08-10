project( RPSF_LogServer )
#日志服务器

File(GLOB SRC_Files *.cpp *.h )


add_executable( RPSF_LogServer ${SRC_Files} )

IF(WIN32)
target_link_libraries(RPSF_LogServer sqlite sigar )
ELSE(WIN32)
target_link_libraries(RPSF_LogServer sqlite sigar -lboost_system -lboost_chrono -lboost_thread -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_serialization -lboost_atomic -lpthread)
ENDIF(WIN32)


