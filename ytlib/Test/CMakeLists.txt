#---------------------------------------------------------------------
project( ytTest )
#使用所有的库进行测试、示例

#aux_source_directory( . SourceFiles)


#---------------------------------------------------------------------
# Construct source groups for nice visualisation in Visual Studio.
File(GLOB Algorithm_Files ../Algorithm/*.h )
source_group( "Algorithm" FILES ${Algorithm_Files} )

File(GLOB Common_Files ../Common/*.h )
source_group( "Common" FILES ${Common_Files} )

File(GLOB DataStructure_Files ../DataStructure/*.h )
source_group( "DataStructure" FILES ${DataStructure_Files} )

File(GLOB FileManager_Files ../FileManager/*.h )
source_group( "FileManager" FILES ${FileManager_Files} )

File(GLOB Interface_Files ../Interface/*.h )
source_group( "Interface" FILES ${Interface_Files} )

File(GLOB LightMath_Files ../LightMath/*.h )
source_group( "LightMath" FILES ${LightMath_Files} )

File(GLOB LogService_Files ../LogService/*.h )
source_group( "LogService" FILES ${LogService_Files} )

File(GLOB NetTools_Files ../NetTools/*.h )
source_group( "NetTools" FILES ${NetTools_Files} )

File(GLOB ProcessManager_Files ../ProcessManager/*.h )
source_group( "ProcessManager" FILES ${ProcessManager_Files} )

File(GLOB SupportTools_Files ../SupportTools/*.h )
source_group( "SupportTools" FILES ${SupportTools_Files} )



File(GLOB Test_Files *.cpp *.h )
source_group( "Test" FILES ${Test_Files} )

#---------------------------------------------------------------------
# Create the elxCommon library.

add_executable( ytTest
  ${Algorithm_Files}
  ${Common_Files}
  ${DataStructure_Files}
  ${FileManager_Files}
  ${Interface_Files}
  ${LightMath_Files}
  ${LogService_Files}
  ${NetTools_Files}
  ${ProcessManager_Files}
  ${SupportTools_Files}
  ${Test_Files}
)

target_link_libraries(ytTest ${YTLIB_LIBRARY} )
