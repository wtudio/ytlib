project( RPSF_DemoPlugin2 )

File(GLOB SRC_Files *.cpp *.h )

ADD_LIBRARY( RPSF_DemoPlugin2 SHARED ${SRC_Files} )
set_target_properties(RPSF_DemoPlugin2 PROPERTIES FOLDER "RPSF") 


