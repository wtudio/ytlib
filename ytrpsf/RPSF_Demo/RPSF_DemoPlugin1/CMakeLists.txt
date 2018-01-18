project( RPSF_DemoPlugin1 )

File(GLOB SRC_Files *.cpp *.h )

ADD_LIBRARY( RPSF_DemoPlugin1 SHARED ${SRC_Files} )
set_target_properties(RPSF_DemoPlugin1 PROPERTIES FOLDER "RPSF") 


