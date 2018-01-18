project( RPSF_DemoPlugin3 )

File(GLOB SRC_Files *.cpp *.h )

ADD_LIBRARY( RPSF_DemoPlugin3 SHARED ${SRC_Files} )
set_target_properties(RPSF_DemoPlugin3 PROPERTIES FOLDER "RPSF") 


