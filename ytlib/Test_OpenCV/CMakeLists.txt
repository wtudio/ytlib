project( ytTest_OpenCV )

File(GLOB CVTools_Files ../CVTools/*.h )
source_group( "CVTools" FILES ${CVTools_Files} )

File(GLOB TestCV_Files *.cpp *.h )
source_group( "TestCV" FILES ${TestCV_Files} )

#---------------------------------------------------------------------
# Create the elxCommon library.

add_executable( ytTest_OpenCV
  ${CVTools_Files}
  ${TestCV_Files}
)

IF(WIN32)
target_link_libraries(ytTest_OpenCV ${OpenCV_LIBS})
ELSE(WIN32)
target_link_libraries(ytTest_OpenCV ${OpenCV_LIBS} -lboost_system -lboost_chrono -lboost_thread -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_serialization -lboost_atomic -lpthread)
ENDIF(WIN32)


