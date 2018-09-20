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

target_link_libraries(ytTest_OpenCV ${YTLIB_LIBRARY} )