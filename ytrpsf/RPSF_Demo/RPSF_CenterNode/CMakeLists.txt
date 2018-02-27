project( RPSF_CenterNode )
#中心节点

File(GLOB SRC_Files *.cpp *.h 
${ytlib_SOURCE_DIR}/ytrpsf/Bus.h 
${ytlib_SOURCE_DIR}/ytrpsf/CenterNode.h 
${ytlib_SOURCE_DIR}/ytrpsf/Msg.h 
${ytlib_SOURCE_DIR}/ytrpsf/Plugin_Bus_Interface.h 
${ytlib_SOURCE_DIR}/ytrpsf/RpsfCfgFile.h 
${ytlib_SOURCE_DIR}/ytrpsf/SysMsg.h 
)


add_executable( RPSF_CenterNode ${SRC_Files} )
set_target_properties(RPSF_CenterNode PROPERTIES FOLDER "RPSF") 

IF(WIN32)
target_link_libraries(RPSF_CenterNode sqlite sigar )
ELSE(WIN32)
target_link_libraries(RPSF_CenterNode sqlite sigar -lboost_system -lboost_chrono -lboost_thread -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_serialization -lboost_atomic -lpthread)
ENDIF(WIN32)


